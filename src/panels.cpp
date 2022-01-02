#include "panels.h"
#include "colors.h"
#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>



/**
 * Constructor initializes the screen to be ready for ncurses
 * And draws the panels
 */
Panels::Panels()
{
    initscr();                                  // Ncurses initialization
    keypad(stdscr, true);
    noecho();
    raw();
    curs_set(0);

    // Check if terminal supports colors
    if (has_colors() == false)
    {
        endwin();
        printf("Your terminal does not support color\n");
		exit(EXIT_FAILURE);
    }
    start_color();
    init_pair(BACKGROUND_COLOR, COLOR_WHITE, COLOR_BLUE);           // Background color
    init_pair(COLUMN_TITLE_COLOR, COLOR_YELLOW, COLOR_BLUE);        // Column titles color
    init_pair(HIGHLIGHT_COLOR, COLOR_BLACK, COLOR_CYAN);            // Menus highlighting color
    init_pair(PATH_COLOR, COLOR_YELLOW, COLOR_BLUE);                // Path displaying color
    init_pair(SORT_MODE_COLOR, COLOR_GREEN, COLOR_BLUE);            // Sort mode display color
    init_pair(OPTIONS_NUMBER_COLOR, COLOR_WHITE, COLOR_BLACK);      // Options colors in menu
    init_pair(OPTIONS_COLOR, COLOR_BLACK, COLOR_CYAN);              // Options colors in menu
    init_pair(OPTIONS_WIN_COLOR, COLOR_BLACK, COLOR_WHITE);         // Window color for options
    init_pair(OPTIONS_TITLE_COLOR, COLOR_GREEN, COLOR_WHITE);       // Title on option window color
    init_pair(ERROR_COLOR, COLOR_RED, COLOR_WHITE);                 // Error color red on white
    init_pair(FORM_COLOR, COLOR_BLACK, COLOR_CYAN);                 // Color for forms (mkdir etc)

    createPanels();                             // Display the panels
}


/**
 * Destructor to destroy windows
 */
Panels::~Panels()
{
    delwin(leftPanel.window);
    delwin(rightPanel.window);
    endwin();
}


/**
 * Draws panels in terminal with size (height, width).
 * Takes care of finding terminal size
 */
void Panels::createPanels()
{
    clear();

    // Initialize PanelWindow struct members
    // Number of Rows & Cols
    int nRows = LINES - 2;
    int nCols = COLS / 2;

    leftPanel.nRows = nRows;
    leftPanel.nCols = nCols;
    rightPanel.nRows = nRows;
    rightPanel.nCols = nCols;

    // Top left corner coordinates
    leftPanel.topLeftCornerY = 1;
    leftPanel.topLeftCornerX = 0;
    rightPanel.topLeftCornerY = 1;
    rightPanel.topLeftCornerX = nCols;

    // Create two new windows
    leftPanel.window = newwin(nRows, nCols, leftPanel.topLeftCornerY, leftPanel.topLeftCornerX);
    rightPanel.window = newwin(nRows, nCols, rightPanel.topLeftCornerY, rightPanel.topLeftCornerX);

    box(leftPanel.window, 0, 0);                   // Draw a box around the windows
    box(rightPanel.window, 0, 0);

    // Draw other designs on border that will be bound to mouse as buttons
    wmove(leftPanel.window, 0, 0);                 // Left arrow that takes you back
    wprintw(leftPanel.window, " <");
    waddch(leftPanel.window, ACS_S7);
    waddch(leftPanel.window, ' ');
    
    wmove(rightPanel.window, 0, 0);                // Left arrow that takes you back
    wprintw(rightPanel.window, " <");
    waddch(rightPanel.window, ACS_S7);
    waddch(rightPanel.window, ' ');

    wmove(leftPanel.window, 0, nCols-5);           // Show hidden folders/files button
    wprintw(leftPanel.window, "[.]");
    
    wmove(rightPanel.window, 0, nCols-5);          // Show hidden folders/files button
    wprintw(rightPanel.window, "[.]");
    
    bkgd(COLOR_PAIR(1));
    wbkgd(leftPanel.window, COLOR_PAIR(BACKGROUND_COLOR));        // Panels background
    wbkgd(rightPanel.window, COLOR_PAIR(BACKGROUND_COLOR));

    drawCols();    

    refresh();
    wrefresh(leftPanel.window); 
    wrefresh(rightPanel.window);
}


/**
 * Deletes windows and calls createPanels()
 */
void Panels::resizePanels()
{
    delwin(leftPanel.window);
    delwin(rightPanel.window);
    createPanels();
}


/**
 * Draws the column lines and titles
 */
void Panels::drawCols()
{
    int nRows = leftPanel.nRows;
    int nCols = leftPanel.nCols;
    
    // Draw bottom line
    mvwhline(leftPanel.window, nRows-3, 1, ACS_HLINE, nCols-2);
    mvwaddch(leftPanel.window, nRows-3, 0, ACS_LTEE);
    mvwaddch(leftPanel.window, nRows-3, nCols-1, ACS_RTEE);
    
    mvwhline(rightPanel.window, nRows-3, 1, ACS_HLINE, nCols-2);
    mvwaddch(rightPanel.window, nRows-3, 0, ACS_LTEE);
    mvwaddch(rightPanel.window, nRows-3, nCols-1, ACS_RTEE);

    // Date Modified Column
    mvwaddch(leftPanel.window, 0, nCols-16, ACS_TTEE);
    mvwvline(leftPanel.window, 1, nCols-16, ACS_VLINE,  nRows-4);
    mvwaddch(leftPanel.window, nRows-3, nCols-16, ACS_BTEE);
    
    mvwaddch(rightPanel.window, 0, nCols-16, ACS_TTEE);
    mvwvline(rightPanel.window, 1, nCols-16, ACS_VLINE,  nRows-4);
    mvwaddch(rightPanel.window, nRows-3, nCols-16, ACS_BTEE);

    // Size column
    mvwaddch(leftPanel.window, 0, nCols-24, ACS_TTEE);
    mvwvline(leftPanel.window, 1, nCols-24, ACS_VLINE,  nRows-4);
    mvwaddch(leftPanel.window, nRows-3, nCols-24, ACS_BTEE);
    
    mvwaddch(rightPanel.window, 0, nCols-24, ACS_TTEE);
    mvwvline(rightPanel.window, 1, nCols-24, ACS_VLINE,  nRows-4);
    mvwaddch(rightPanel.window, nRows-3, nCols-24, ACS_BTEE);

    // Add Column titles
    wattron(leftPanel.window, COLOR_PAIR(COLUMN_TITLE_COLOR) | A_BOLD);
    mvwprintw(leftPanel.window, 1, (nCols-24)/2 - 2, "Name");
    mvwprintw(leftPanel.window, 1, nCols-24 + 2, "Size");
    mvwprintw(leftPanel.window, 1, nCols-16 + 2, "Modify Time");
    wattroff(leftPanel.window, COLOR_PAIR(COLUMN_TITLE_COLOR) | A_BOLD);
    
    wattron(rightPanel.window, COLOR_PAIR(COLUMN_TITLE_COLOR) | A_BOLD);
    mvwprintw(rightPanel.window, 1, (nCols-24)/2 - 2, "Name");
    mvwprintw(rightPanel.window, 1, nCols-24 + 2, "Size");
    mvwprintw(rightPanel.window, 1, nCols-16 + 2, "Modify Time");
    wattroff(rightPanel.window, COLOR_PAIR(COLUMN_TITLE_COLOR) | A_BOLD);
}