#ifndef PANELS_H
#define PANELS_H

#include <ncurses.h>


class Panels

/**
 * This class is for the design elements of the panels only.
 * Takes care of the Ncurses printing of panels in terminal.
 * Resizing is supported. 
 */

{
protected:
    struct PanelWindow
    {
        WINDOW* window;                             // Left or Right panel of split screen of WINDOWS* type
        int nRows;                                  // Height of the window
        int nCols;                                  // Width of the window
        int topLeftCornerY;                         // Y coordinate of top left corner
        int topLeftCornerX;                         // X coordinate of top left corner
    };

    PanelWindow leftPanel;                          // Left panel struct
    PanelWindow rightPanel;                         // Right panel struct

    void createPanels();                            // Draws panels in terminal. Takes care of finding terminal size    
    void resizePanels();                            // Deletes windows and calls createPanels()
    void drawCols();                                // Draws the column lines and titles

public:
    Panels();
    ~Panels();
};


#endif