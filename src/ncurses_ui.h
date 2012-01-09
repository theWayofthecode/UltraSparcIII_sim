/**
 * \file ncurses_ui.h
 * The ncurses_ui implements the user interface based on ncurses library. Once you
 * initialize by calling init_ncurses() you can use the nprintf function which is
 * similar to printf family with the only difference the first parameter of type int
 * which denotes the subwindow starting from 0 = leftmost. Before terminates the program
 * the end_ncurses() have to be called to clean up.
 */

#ifndef __NCURSES_UI_H_
#define __NCURSES_UI_H_

/**
 * If you want to disable ncurses (enable normal printfs) uncomment the
 * following define
 */
//#define NORMAL_OUTPUT

/**
 * Initializes ncurses
 * \param[in] num The number of subwindows
 * \param[in] titles The array of strings(titles) from left to right
 */
void init_ncurses(int num, char **titles);

/**
 * Prints to a subwindow
 * \param[in] idx The id of subwindow from left = 0 to right
 */
void nprintf(int idx, char *fmt, ...);

/**
 * Clear and redraw all windows
 */
void nreset_all();

/**
 * Clear the given window
 * From 0 to #ofwindows - 1
 * from left to right
 */
void nreset(int w);

/**
 * Free the resouces
 */
void end_ncurses();

/**
 * Init keyboard controls
 * \param[in] key_func pairs to register functionality
 */
void init_key_ctrls(int key_func_num, ...);

#endif
