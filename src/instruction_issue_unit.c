#include <stdio.h>
#include <stdlib.h>

#include "ncurses_ui.h"
#include "cpu.h"
#include "instruction_issue_unit.h"

void *instruction_issue(void *arg)
{
    while (1) {
        clk_cycle();
        nprintf(0, "iiu\n");
    }
}

