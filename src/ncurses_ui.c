/**
 * \file ncurses_ui.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ncurses.h>
#include <pthread.h>

#include "ncurses_ui.h"


typedef struct key_func{
    int key;
    void (*func)(void);
    struct key_func *next;
}key_func;

WINDOW **subwins;

int g_num_of_wins;

void *key_ctrl_loop(void *arg);

void nprintf(int idx, char *fmt, ...)
{
    va_list varg;
    va_start(varg, fmt);
#ifndef NORMAL_OUTPUT
    vwprintw(subwins[idx], fmt, varg);
    wrefresh(subwins[idx]);
#else
    vprintf(fmt, varg);
#endif
    va_end(varg);
}

void init_ncurses(int num, char **titles)
{
#ifndef NORMAL_OUTPUT
    g_num_of_wins = num;
    int height, width, i;
    WINDOW **wins; 
    wins = (WINDOW **)calloc(num, sizeof(WINDOW *));
    subwins = (WINDOW **)calloc(num, sizeof(WINDOW *));
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    height = LINES;
    width = COLS / num;
    for (i = 0; i < num; i++) {
        wins[i] = newwin(height, width, 0, i * width);
        subwins[i] = derwin(wins[i], height - 2, width - 2, 1, 1);
        box(wins[i], 0, 0);
        scrollok(subwins[i], TRUE);
        wprintw(wins[i], "+----%s", titles[i]);
        wrefresh(wins[i]);
    }
#endif
}

void refresh_all()
{
#ifndef NORMAL_OUTPUT
    int i;
    for(i = 0; i < g_num_of_wins; i++) {
        touchwin(subwins[i]);
        wrefresh(subwins[i]);
    }
#endif
}


void end_ncurses()
{
#ifndef NORMAL_OUTPUT
    endwin();
#endif
}

void init_key_ctrls(int key_func_num, ...)
{
    va_list ap;
    int i;
    key_func *root, *new;
    root = (key_func *)malloc(sizeof(key_func)); //sentinel node
    root->next = NULL;

    va_start(ap, key_func_num);
    for (i = 0; i < key_func_num; i++) {
        new = (key_func *)malloc(sizeof(key_func));
        new->key = va_arg(ap, int);
        new->func = va_arg(ap, void (*)(void));
        new->next = root->next;
        root->next = new;
    }
    va_end(ap);

    pthread_t id;
    pthread_create(&id, NULL, key_ctrl_loop, (void *)root->next);
    free(root); //free sentinel
}

void *key_ctrl_loop(void *arg)
{
    key_func *it;
    char cmd;
    while (1) {
#ifndef NORMAL_OUTPUT
        cmd = wgetch(subwins[0]);
#else
        cmd = getchar();
#endif
        for (it = (key_func *)arg; it != NULL; it = it->next) {
            if (it->key == cmd)
                it->func();
        }
    }
}
        
