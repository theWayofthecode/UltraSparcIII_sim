#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ncurses_ui.h"
#include "cpu.h"
#include "instruction_issue_unit.h"

/**
 * Instruction data structure.
 * Field description [SPARCv9 p.87]
 */
typedef struct instruct { //Format3. The other with unions.
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
    struct instruct *next;
} instruct;

/**
 * Globals.
 * All identifiers have _ prefix by convention.
 */

// Execution stages function array 
static void (*_exec_stages[6])(void);

// Assembly code source file
FILE *_src;

// Current instruction in process
instruct *_ci = NULL;

//Instruction Queue
instruct *_queue = NULL;

static void init(char *file_name);

/**
 * Instruction issue unit execution
 * stages.
 */
//A
void address_generation();
//P
void instruction_prefetch();
//F
void instruction_fetch();
//B
void branch_target_calculation();
//I
void instruction_decode();
//J
void instruction_steer();

/**
 * Instruction decoders
 */
void dec_add();
int dec_reg(char *reg);

/**
 * Helper functions
 */

instruct *new_instruction();
void del_instruction(instruct *in);
int enqueue(instruct *in);
void dequeue(instruct *in);
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
        stage = (stage + 1) % 6;
    }
}

void init(char *file_name)
{
    _exec_stages[0] = address_generation;
    _exec_stages[1] = instruction_prefetch;
    _exec_stages[2] = instruction_fetch;
    _exec_stages[3] = branch_target_calculation;
    _exec_stages[4] = instruction_decode;
    _exec_stages[5] = instruction_steer;

    _src = fopen(file_name, "r");
    assert(_src != NULL);

    _queue = new_instruction(); //sentinel node
    _queue->i = 0; //number of instruction nodes
    _queue->next = _queue;
}

/**
 * Execution stages
 */
void address_generation()
{
    fprintf(stderr, "[IIU]:Address generation\n");
}

void instruction_prefetch()
{
    fprintf(stderr, "[IIU]:Instruction prefetch\n");
}

void instruction_fetch()
{
    char str[32];
    fprintf(stderr, "[IIU]:Instruction fetch\n");

    if (feof(_src))
        return;
    
    fscanf(_src, "%[^\n]\n", str);
    _ci = new_instruction();
    _ci->string = strdup(str);
}

void branch_target_calculation()
{
    fprintf(stderr, "[IIU]:Branch target calculation\n");
}

void instruction_decode()
{
    char cmd[16];
    int items = 0;

    fprintf(stderr, "[IIU]:Instruction decode\n");
    if (_ci == NULL)
        return;

    items = sscanf(_ci->string, "%s", cmd); 
    assert(items == 1);

    if (!(strcmp(cmd, "ADD"))) {
        dec_add();
    } else {
        assert(0);
    }

    if (!enqueue(_ci))
       fprintf(stderr, "[IIU] queue is full\n"); 
}

void instruction_steer()
{
    fprintf(stderr, "[IIU]:Instruction steer\n");
}

/**
 * Instruction Decoders
 */

//SPARCv9 [p.160]
void dec_add()
{
    char cmd[16];
    char op1[16];
    char op2[16];
    char op3[16];
    int items;

    items = sscanf(_ci->string, "%s %s %s %s", cmd, op1, op2, op3); 
    assert(items == 4);

    _ci->op = 2;
    _ci->op3 = 0;
    _ci->rs1 = dec_reg(op1);
    _ci->rd = dec_reg(op3);
    if (op2[0] == '$') {
        _ci->i = 0;
        _ci->rs2 = dec_reg(op2);
    } else {
        _ci->i = 1;
        _ci->simm13 = atoi(op2);
    }
}

//SPARCv9 [p.49]
int dec_reg(char *reg)
{
    int ireg;

    assert(reg[0] == '$');

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

void del_instruction(instruct *in)
{
    free(in->string);
    free(in);
}

int enqueue(instruct *in)
{
    instruct *it;

    if (_queue->i > 19 || in == NULL) //queue is full
        return 0;

    for(it = _queue; it->next != _queue; it = it->next) {
        if (it->next == in) //already in queue
            return 1;
    }
    in->next = it->next;
    it->next = in;
    _queue->i++;

    return 1;
}

void dequeue(instruct *in)
{
    instruct *it;

    for(it = _queue; it->next != in; it = it->next);
    it->next = in->next;
    del_instruction(in);
    _queue->i--;
}

void disp_queue()
{
    int i;
    instruct *in;

    nreset(0); //clear the IIU subwindow
    in = _queue->next;
    for (i = 0; i < _queue->i; i++) {
        nprintf(0, "%d] %s\n", i, in->string);
        in = in->next;
    }
}
