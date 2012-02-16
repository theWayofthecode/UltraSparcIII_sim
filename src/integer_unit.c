#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ncurses_ui.h"
#include "cpu.h"
#include "integer_unit.h"

/**
 * Globals.
 * All identifiers have _ prefix by convention.
 */

// Execution stages function array 
static void (*_exec_stages[8])(void);

static void init();

/**
 * Integer unit execution
 * stages.
 */
//E
void execute();
//C
void data_cache_access();
//M
void memory_bypass();
//W
void working_register_file_write();
//X
void pipe_extend();
//T
void trap();
//D
void done();

/**
 * Main
 */
void *integer(void *arg)
{
    int stage;

    init();
    stage = 0;
    while (1) {
        _exec_stages[stage]();
        
        //Loop logistics
        clk_cycle();
        stage = (stage + 1) % 7;
    }
}

void init()
{
    _exec_stages[0] = execute;
    _exec_stages[1] = data_cache_access;
    _exec_stages[2] = memory_bypass;
    _exec_stages[3] = working_register_file_write;
    _exec_stages[4] = pipe_extend;
    _exec_stages[5] = trap;
    _exec_stages[6] = done;
}

/**
 * Execution stages
 */

void execute()
{
    fprintf(stderr, "[IU]: Integer execution\n");
}

void data_cache_access()
{
    fprintf(stderr, "[IU]: Data cache access\n");
}

void memory_bypass()
{
    fprintf(stderr, "[IU]: Memory bypass\n");
}

void working_register_file_write()
{
    fprintf(stderr, "[IU]: Working register file write\n");
}

void pipe_extend()
{
    fprintf(stderr, "[IU]: Pipe extend\n");
}

void trap()
{
    fprintf(stderr, "[IU]: Trap\n");
}

void done()
{
    fprintf(stderr, "[IU]: Done\n");
}

