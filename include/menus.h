#ifndef MENUS_H
#define MENUS_H

#include "panels.h"
#include "file.h"
#include "directoryReader.h"
#include "linkedList.h"
#include <ncurses.h>


class Menus : protected Panels
/**
 * This class displays all the files in both panels
 * It inherits the Panels class which takes care of drawing
 * and resizing the panels.
 */

{

protected:  
    enum sortMode                                           // Enum that defines what sort mode is running
    {
        byName,
        bySize,
        byTimeModified
    };

    struct MenuWindow   
    {
        WINDOW* window;                             // Left or Right menu of split screen of WINDOWS* type
        WINDOW* nameDisplayWindow;                  // Displays name of the file that is highlighted
        WINDOW* pathDisplayWindow;                  // Displays the path on top of each panel
        DirectoryReader* directoryReader;           // Each menu has a directory reader to read the files in current folder
        LinkedList history;                         // This linked list keeps track of the history of files we've been to.
        PanelWindow* panelWindow;                   // Which panel this menu corresponds to
        int nRows;                                  // Height of the window
        int nCols;                                  // Width of the window
        int topLeftCornerY;                         // Y coordinate of top left corner
        int topLeftCornerX;                         // X coordinate of top left corner      
        int colPos_1;                               // X coordinate of cursor to type in 1st column
        int colPos_2;                               // X coordinate of cursor to type in 2nd column
        int colPos_3;                               // X coordinate of cursor to type in 3rd column
        int item_ptr;                               // Which item on the menu should be highlighted (index starts from 0)
        int scrollOffset;                           // How many items to skip before printing first item on first line
        bool showHidden;                            // Whether to show hidden files or not
        sortMode itemSortMode;                      // Sort by name, size or date modified
    };  

    MenuWindow leftMenu;                            // Left menu struct
    MenuWindow rightMenu;                           // Right menu struct
    MenuWindow* currentMenu;                        // Which menu the user is using and should be highlighted

    void menuEventLoop(int input);                  // Main loop that keeps the menus running
    void highlighter(int mode, int row);            // Highlights or unhighlights row
    void resizeMenus();                             // Resize handler for menus. Draws menus according to terminal size
    char* formatNameWithQuotes(char* name);         // Formats file names which have spaces so that they can be opened in terminals
    void printMenuItems(MenuWindow* menuWindow);    // Prints out the files and directories in the menus
    void sortItems(MenuWindow* menuWindow, sortMode mode);  // Sorts items in current menu
    void displayFileName(MenuWindow* menuWindow);           // Displays the name of the file in the bottom bar


    
private:
    void createMenus();                                     // Draws menus in terminal. Takes care of finding terminal size
    char* getFormattedItemName(File* file, int maxLen);     // Check if name is longer than column width and split name into two halves so we can see extension of the file
    void openFile();                                        // Opens different types of files
    void displayPath(MenuWindow* menuWindow);               // Displays the path at the top of the window
    char* formatPath(char* path);                           // Formats the current path so that we can display it on top of the menus
    void showSortMode(MenuWindow* menuWindow);              // Displays the type of sorting being use (by name/size/time modified)
    
public:
    Menus(char* rootDir);
    ~Menus();
};


#endif