#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ncurses_ui.h"
#include "cpu.h"
#include "instruction_issue_unit.h"

/**
 * Pipeline codes
 */
enum pipeline {
    A0,
    A1,
    BR,
    MS,
    FGM,
    FGA,
    NONE
};

/**
 * Instruction data structure.
 * Field description [SPARCv9 p.87]
 */
typedef struct instruct{ //Format3. The other with unions.
    int op;
    int rd;
    int op3;
    int rs1;
    int i;
    union {
        int rs2;
        int simm13;
    };
    
    /* Sim related */
    //Instruction's string representation
    char *string;
} instruct;

/**
 * Instruction group of 4
 */
typedef struct group{
    instruct *inst[4];
    enum pipeline pipe[4];

    struct group *next;
} group;

/**
 * Globals.
 * All identifiers have _ prefix by convention.
 */

// Execution stages function array 
static void (*_exec_stages[7])(void);

// Assembly code source file
FILE *_src;

// Current instruction in process
instruct *_ci[4];

//Instruction Queue
group *_queue = NULL;
int items_in_Q;

static void init(char *file_name);

/**
 * Instruction issue unit execution
 * stages.
 */
//A
void address_generation();
//P
void preliminary_fetch();
//F
void fetch();
//B
void branch_target_computation();
//I
void instruction_group_formation();
//J
void instruction_group_staging();
//R
void dispatch_and_register_access();

/**
 * Instruction decoders
 */
void dec_add();
int dec_reg(char *reg);

/**
 * Helper functions
 */

instruct *new_instruction();
group *new_group();
void del_instruction(instruct *in);
void del_group(group *in);
int enqueue(group *in);
group *dequeue();
void disp_queue();

/**
 * Main
 */
void *instruction_issue(void *arg)
{
    int stage;

    init((char *)arg);
    
    stage = 0;
    while (1) {
        _exec_stages[stage]();
        disp_queue();
        
        //Loop logistics
        clk_cycle();
        stage = (stage + 1) % 7;
    }
}

void init(char *file_name)
{
    int i;

    _exec_stages[0] = address_generation;
    _exec_stages[1] = preliminary_fetch;
    _exec_stages[2] = fetch;
    _exec_stages[3] = branch_target_computation;
    _exec_stages[4] = instruction_group_formation;
    _exec_stages[5] = instruction_group_staging;
    _exec_stages[6] = dispatch_and_register_access;

    _src = fopen(file_name, "r");
    assert(_src != NULL);

    _queue = new_group(); //sentinel node
    items_in_Q = 0;
    _queue->next = _queue;

    for (i = 0; i < 4; i++) {
        _ci[i] = NULL;
    }
}

/**
 * Instruction unit stages
 */

//A
void address_generation()
{
    fprintf(stderr, "[IIU]:Address generation\n");
}

//P
void preliminary_fetch()
{
    fprintf(stderr, "[IIU]:Instruction prefetch\n");
}

//F
void fetch()
{
    int i;
    char str[32];
    fprintf(stderr, "[IIU]:Instruction fetch\n");

    if (feof(_src))
        return;
   
    for (i = 0; i < 4; i++) {
        fscanf(_src, "%[^\n]\n", str);
        _ci[i] = new_instruction();
        _ci[i]->string = strdup(str);
    }
}

//B
void branch_target_computation()
{
    fprintf(stderr, "[IIU]:Branch target calculation\n");
}

//I
void instruction_group_formation()
{
    int i, j = 0;
    char cmd[16];
    int items = 0;
    group *gp = new_group();
    enum pipeline av_pipe = A0;

    fprintf(stderr, "[IIU]:Instruction decode\n");

    for (i = 0; i < 4; i++) {
        if (_ci[i] == NULL)
            return;

        items = sscanf(_ci[i]->string, "%s", cmd); 
        assert(items == 1);

        if (!(strcmp(cmd, "ADD"))) {
            dec_add(_ci[i]);
            gp->inst[j] = _ci[i];
            gp->pipe[j] = av_pipe;
            av_pipe++;
            j++;
        } else {
            assert(0);
        }

        if (av_pipe != A0 && av_pipe != A1) {
            if (!enqueue(gp))
               fprintf(stderr, "[IIU] queue is full\n"); 
            gp = new_group();
            av_pipe = A0;
            j = 0;
        }
    }
}

//J
void instruction_group_staging()
{
    fprintf(stderr, "[IIU]:Instruction steer\n");
}

//R
void dispatch_and_register_access()
{
    fprintf(stderr, "[IIU]:Dispatch and register access\n");
}
    

/**
 * Instruction Decoders
 */

//SPARCv9 [p.160]
void dec_add(instruct *inst)
{
    char cmd[16];
    char op1[16];
    char op2[16];
    char op3[16];
    int items;

    items = sscanf(inst->string, "%s %s %s %s", cmd, op1, op2, op3); 
    assert(items == 4);

    inst->op = 2;
    inst->op3 = 0;
    inst->rs1 = dec_reg(op1);
    inst->rd = dec_reg(op3);
    if (op2[0] == '%') {
        inst->i = 0;
        inst->rs2 = dec_reg(op2);
    } else {
        inst->i = 1;
        inst->simm13 = atoi(op2);
    }
}

//SPARCv9 [p.49]
int dec_reg(char *reg)
{
    int ireg;

    assert(reg[0] == '%');

    ireg = reg[2] - 48; //ascii to int
    if (reg[1] == 'g') {
        ireg += 0;
    } else if (reg[1] == 'l') {
        ireg += 8;
    } else if (reg[1] == 'o') {
        ireg += 16;
    } else {
        assert(0);
    }
    return ireg;
}

/**
 * helpers
 */

instruct *new_instruction()
{
    instruct *new;

    new = (instruct *)malloc(sizeof(instruct));
    assert(new != NULL);

    new->op = -1;
    new->rd = -1;
    new->op3 = -1;
    new->rs1 = -1;
    new->i = -1;
    new->rs2 = -1;
    new->string = NULL;

    return (new);
}

group *new_group()
{
    int i;
    group *new;

    new = (group *)malloc(sizeof(group));
    assert(new != NULL);

    for (i = 0; i < 4; i++) {
        new->inst[i] = NULL;
        new->pipe[i] = NONE;
    }

    return new;
}

void del_instruction(instruct *in)
{
    free(in->string);
    free(in);
}

void del_group(group *in)
{
    int i;

    for (i = 0; i < 4; i++)
        del_instruction(in->inst[i]);
        
    free(in);
}

int enqueue(group *in)
{
    group *it;

    if (items_in_Q >= 4 || in == NULL) //queue is full
        return 0;

    for(it = _queue; it->next != _queue; it = it->next) {
        if (it->next == in) //already in queue
            return 1;
    }
    in->next = it->next;
    it->next = in;

    items_in_Q++;
    return 1;
}

group *dequeue()
{
    group *gp;

    gp = _queue->next;
    _queue->next = gp->next; 
    gp->next = NULL;
    items_in_Q--;
    return gp;
}

void disp_queue()
{
    int i, j;
    group *gp;

    nreset(0); //clear the IIU subwindow
    gp = _queue->next;
    for (i = 0; i < items_in_Q; i++) {
        for (j = 0; j < 4; j++) {
            if (gp->inst[j] != NULL) {
                nprintf(0, "%d] %s, pl{%d}\n", i, gp->inst[j]->string, gp->pipe[j]);
            } else {
                nprintf(0, "%d] NONE\n", i);
            }
        }
        gp = gp->next;
    }
}
