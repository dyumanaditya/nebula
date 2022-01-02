#include "fileManager.h"
#include "keybindings.h"
#include "colors.h"
#include <ncurses.h>
#include <sys/stat.h>
#include <form.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>



FileManager::FileManager(char* rootDir) : Menus(rootDir), currentOption(none)
{
    createOptions();
}


FileManager::~FileManager()
{
}


/**
 * Main loop that keeps the program running 
 */
void FileManager::mainEventLoop()
{
    bool inLoop = true;

    MEVENT mouseEvent;
    int keyboardInput;
    raw();
    while (inLoop)
    {
        keyboardInput = getch();
        switch (keyboardInput)
        {

        /**
         * Some mouse event has occured
         */
        case KEY_MOUSE:
        {
            if (getmouse(&mouseEvent)==OK)
            {
                MenuWindow* menuInFocus;
                MenuWindow* menuNotInFocus;
                menuInFocus = currentMenu;
                menuNotInFocus = (menuInFocus == &leftMenu) ? &rightMenu : &leftMenu;

                // Calculate how many items there are in the menu
                int menuInFocus_itemLen;
                int menuNotInFocus_itemLen;
                if (menuInFocus->showHidden)
                {
                    menuInFocus_itemLen = menuInFocus->directoryReader->filesAndHiddenFiles.getLength();
                }
                else
                {
                    menuInFocus_itemLen = menuInFocus->directoryReader->files.getLength();
                }
                if (menuNotInFocus->showHidden)
                {
                    menuNotInFocus_itemLen = menuNotInFocus->directoryReader->filesAndHiddenFiles.getLength();
                }
                else
                {
                    menuNotInFocus_itemLen = menuNotInFocus->directoryReader->files.getLength();
                }


                // Check for single click inside current window
                if (mouseEvent.bstate & BUTTON1_CLICKED && wenclose(menuInFocus->window, mouseEvent.y, mouseEvent.x))
                {
                    wmouse_trafo(menuInFocus->window, &mouseEvent.y, &mouseEvent.x, FALSE);

                    if (mouseEvent.y + menuInFocus->scrollOffset < menuInFocus_itemLen)
                    {
                        highlighter(1, menuInFocus->item_ptr - menuInFocus->scrollOffset);
                        highlighter(0, mouseEvent.y);
                        menuInFocus->item_ptr = mouseEvent.y + menuInFocus->scrollOffset;
                        displayFileName(menuInFocus);
                    }
                }
                
                // Single click on other menu to change focus to that menu
                else if (mouseEvent.bstate & BUTTON1_CLICKED && wenclose(menuNotInFocus->window, mouseEvent.y, mouseEvent.x))
                {
                    menuEventLoop(KEY_STAB);
                    menuInFocus = currentMenu;
                    menuNotInFocus = (menuInFocus == &leftMenu) ? &rightMenu : &leftMenu;

                    wmouse_trafo(menuInFocus->window, &mouseEvent.y, &mouseEvent.x, FALSE);

                    if (mouseEvent.y + menuInFocus->scrollOffset < menuNotInFocus_itemLen)
                    {
                        highlighter(1, menuInFocus->item_ptr - menuInFocus->scrollOffset);
                        highlighter(0, mouseEvent.y);
                        menuInFocus->item_ptr = mouseEvent.y + menuInFocus->scrollOffset;
                        displayFileName(menuInFocus);
                    }
                }

                // Check for double click in current window: highlight and open
                else if (mouseEvent.bstate & BUTTON1_DOUBLE_CLICKED && wenclose(menuInFocus->window, mouseEvent.y, mouseEvent.x))
                {
                    wmouse_trafo(menuInFocus->window, &mouseEvent.y, &mouseEvent.x, FALSE);

                    // Sleep for a very short time to display new highlight
                    timespec t1, t2;
                    t1.tv_sec = 0;
                    t1.tv_nsec = 100000000L;

                    if (mouseEvent.y + menuInFocus->scrollOffset < menuInFocus_itemLen)
                    {
                        highlighter(1, menuInFocus->item_ptr - menuInFocus->scrollOffset);
                        highlighter(0, mouseEvent.y);
                        menuInFocus->item_ptr = mouseEvent.y + menuInFocus->scrollOffset;
                        nanosleep(&t1, &t2);
                        menuEventLoop(KEY_ENTER);
                    }
                }

                // Check for double click in other window: highlight and open
                else if (mouseEvent.bstate & BUTTON1_DOUBLE_CLICKED && wenclose(menuNotInFocus->window, mouseEvent.y, mouseEvent.x))
                {
                    menuEventLoop(KEY_STAB);
                    menuInFocus = currentMenu;
                    menuNotInFocus = (menuInFocus == &leftMenu) ? &rightMenu : &leftMenu;

                    wmouse_trafo(menuInFocus->window, &mouseEvent.y, &mouseEvent.x, FALSE);

                    // Sleep for a very short time to display new highlight
                    timespec t1, t2;
                    t1.tv_sec = 0;
                    t1.tv_nsec = 100000000L;

                    if (mouseEvent.y + menuInFocus->scrollOffset < menuNotInFocus_itemLen)
                    {
                        highlighter(1, menuInFocus->item_ptr - menuInFocus->scrollOffset);
                        highlighter(0, mouseEvent.y);
                        menuInFocus->item_ptr = mouseEvent.y + menuInFocus->scrollOffset;
                        nanosleep(&t1, &t2);
                        menuEventLoop(KEY_ENTER);
                    }
                }

                // Check for scrolling
                else if (mouseEvent.bstate & BUTTON4_PRESSED && wenclose(menuInFocus->window, mouseEvent.y, mouseEvent.x))
                {
                    menuEventLoop(KEY_UP);
                }

                else if (mouseEvent.bstate & BUTTON5_PRESSED && wenclose(menuInFocus->window, mouseEvent.y, mouseEvent.x))
                {
                    menuEventLoop(KEY_DOWN);
                }


                // Check for single clicks on column headers on top or hidden and back buttons
                else if (mouseEvent.bstate & BUTTON1_CLICKED && wenclose(menuInFocus->panelWindow->window, mouseEvent.y, mouseEvent.x))
                {
                    wmouse_trafo(menuInFocus->panelWindow->window, &mouseEvent.y, &mouseEvent.x, FALSE);

                    // Check for click on Name/Size/Time modified to sort the list accordingly
                    if (mouseEvent.y==1)
                    {
                        int nameColPos = (menuInFocus->panelWindow->nCols-24)/2 - 2;
                        int sizeColPos = menuInFocus->panelWindow->nCols-24 + 2;
                        int timeModifiedColPos = menuInFocus->panelWindow->nCols-16 + 2;
                        if (nameColPos <= mouseEvent.x && mouseEvent.x <= nameColPos+4)
                        {
                            menuEventLoop(SORT_BY_NAME);
                        }
                        else if (sizeColPos <= mouseEvent.x && mouseEvent.x <= sizeColPos+4)
                        {
                            menuEventLoop(SORT_BY_SIZE);
                        }
                        else if (timeModifiedColPos <= mouseEvent.x && mouseEvent.x <= timeModifiedColPos+10)
                        {
                            menuEventLoop(SORT_BY_TIME_MODIFIED);
                        }
                    }

                    // Check for click on hidden buttons or click on back button
                    else if (mouseEvent.y==0)
                    {
                        int hiddenButtonPos = menuInFocus->panelWindow->nCols-5;
                        int backButtonPos = 0;
                        if (hiddenButtonPos <= mouseEvent.x && mouseEvent.x <= hiddenButtonPos+2)
                        {
                            menuEventLoop(KEY_HIDDEN);
                        }
                        else if (backButtonPos <= mouseEvent.x && mouseEvent.x <= backButtonPos+2)
                        {
                            menuEventLoop(KEY_BACK);
                        }
                    }
                }

                // Check for single click on options below
                else if ((mouseEvent.bstate & BUTTON1_CLICKED || mouseEvent.bstate & BUTTON1_DOUBLE_CLICKED))
                {
                    mouse_trafo(&mouseEvent.y, &mouseEvent.x, FALSE);
                    if (mouseEvent.y==LINES-1)
                    {
                        int helpColPos = 0;
                        int infoColPos = strlen(options[0]) + optionsPadding;
                        int copyColPos = infoColPos + strlen(options[1]) + optionsPadding;
                        int pasteColPos = copyColPos + strlen(options[2]) + optionsPadding;
                        int cutColPos = pasteColPos + strlen(options[3]) + optionsPadding;
                        int deleteColPos = cutColPos + strlen(options[4]) + optionsPadding;
                        int mkdirColPos = deleteColPos + strlen(options[5]) + optionsPadding;
                        int renameColPos = mkdirColPos + strlen(options[6]) + optionsPadding;
                        int attribColPos = renameColPos + strlen(options[7]) + optionsPadding;
                        int quitColPos = attribColPos + strlen(options[8]) + optionsPadding;

                        // Help option
                        if (helpColPos <= mouseEvent.x && mouseEvent.x <= helpColPos+strlen(options[0]))
                        {
                            helpLoop();
                        }

                        // Info option
                        if (infoColPos <= mouseEvent.x && mouseEvent.x <= infoColPos+strlen(options[1]))
                        {
                            infoLoop();
                        }

                        // Copy option
                        else if (copyColPos <= mouseEvent.x && mouseEvent.x <= copyColPos+strlen(options[2]))
                        {
                            copyItem();
                        }

                        // Paste option
                        else if (pasteColPos <= mouseEvent.x && mouseEvent.x <= pasteColPos+strlen(options[3]))
                        {
                            pasteItem();
                        }

                        // Cut option
                        else if (cutColPos <= mouseEvent.x && mouseEvent.x <= cutColPos+strlen(options[4]))
                        {
                            copyItem();
                            currentOption = cutOption;
                        }

                        // Delete option
                        else if (deleteColPos <= mouseEvent.x && mouseEvent.x <= deleteColPos+strlen(options[5]))
                        {
                            deleteLoop();
                        }

                        // Mkdir option
                        else if (mkdirColPos <= mouseEvent.x && mouseEvent.x <= mkdirColPos+strlen(options[6]))
                        {
                            mkdirLoop();
                        }

                        // Rename option
                        else if (renameColPos <= mouseEvent.x && mouseEvent.x <= renameColPos+strlen(options[7]))
                        {
                            renameLoop();
                        }

                        // Attributes option
                        else if (attribColPos <= mouseEvent.x && mouseEvent.x <= attribColPos+strlen(options[8]))
                        {
                            attributeLoop();
                        }

                        // Quit option
                        else if (quitColPos <= mouseEvent.x && mouseEvent.x <= quitColPos+strlen(options[9]))
                        {
                            inLoop = false;
                        }
                    }

                }
            }
            break;
        }

        /**
         * Terminal resize has occured
         * Draw all panels and menus again
         */
        case KEY_RESIZE:
        {
            resizeFileManager();
            break;
        }

        /**
         * 2 shortcuts for help
         */
        case KEY_HELP: case KEY_F(1):
        {
            helpLoop();
            break;
        }

        /**
         * 2 shortcuts for info
         */
        case KEY_INFO: case KEY_F(2):
        {
            infoLoop();
            break;
        }

        /**
         * 2 shortcuts for copying
         */
        case KEY_COPY: case KEY_F(3):
        {
            copyItem();
            break;
        }

        /**
         * 2 shortcuts for pasting
         */
        case KEY_PASTE: case KEY_F(4):
        {
            pasteItem();
            break;
        }

        /**
         * 2 shortcuts for cutting
         */
        case KEY_CUT: case KEY_F(5):
        {
            copyItem();
            currentOption = cutOption;
            break;
        }

        /**
         * 2 shortcuts for deleting
         */
        case KEY_DELETE: case KEY_F(6):
        {
            deleteLoop();
            break;
        }

        /**
         * 2 shortcuts for mkdir
         */
        case KEY_MKDIR: case KEY_F(7):
        {
            mkdirLoop();
            break;
        }


        /**
         * 2 shortcuts for rename
         */
        case KEY_RENAME: case KEY_F(8):
        {
            renameLoop();
            break;
        }

        case KEY_ATTRIB: case KEY_F(9):
        {
            attributeLoop();
            break;
        }

        /**
         * Exit program
         */
        case KEY_QUIT: case KEY_F(10):
        {
            inLoop = false;
            break;
        }
        
        /**
         * Send all other keyboard inputs to menus
         */
        default:
            menuEventLoop(keyboardInput);
            break;
        }
    }
     
}


/**
 * Resizes everything according to new dimensions
 */
void FileManager::resizeFileManager()
{
    resizePanels();
    resizeMenus();
    createOptions();
}


void FileManager::createOptions()
{
    // 1 Help   2 Info   3 Copy    4 Paste   5 Cut   6 Move  7 Delete    8 Mkdir   9 Rename    10 Quit

    strcpy(options[0], "1Help ");
    strcpy(options[1], "2Info ");
    strcpy(options[2], "3Copy ");
    strcpy(options[3], "4Paste ");
    strcpy(options[4], "5Cut ");
    strcpy(options[5], "6Delete ");
    strcpy(options[6], "7Mkdir ");
    strcpy(options[7], "8Rename ");
    strcpy(options[8], "9Attrib ");
    strcpy(options[9], "10Quit ");

    optionsPadding = 0;
    int minSpacing = 76;
    move(LINES-1, 0);

    // If terminal size is enough to fit everything
    if (COLS>=minSpacing)
    {
        // Calculate padding
        optionsPadding = (int) (COLS-minSpacing)/10 + 1;
    }

    for (int i=0; i<10; ++i)
    {
        attron(COLOR_PAIR(OPTIONS_NUMBER_COLOR));
        printw("%c", options[i][0]);
        if (i==9)
        {
            printw("%c", options[i][1]);
        }
        attroff(COLOR_PAIR(OPTIONS_NUMBER_COLOR));

        attron(COLOR_PAIR(OPTIONS_COLOR));
        if (i==9)
        {
            printw("%s", options[i]+2);
        }
        else
        {
            printw("%s", options[i]+1);
        }
        
        for (int j=0; j<optionsPadding; ++j)
        {
            printw(" ");
        }
        attroff(COLOR_PAIR(OPTIONS_COLOR));
    }
    refresh();
}


void FileManager::copyItem()
{
    // Cannot operate on ".."
    if (currentMenu->item_ptr==0)
    {
        errorLoop("Cannot operate on \"..\" folder");
        currentOption = none;
        return;
    }

    strcpy(copyPathBuffer, currentMenu->directoryReader->currentPath);
    strcat(copyPathBuffer, "/");
    if (currentMenu->showHidden)
    {
        strcpy(copyFileNameBuffer, currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->name);
        strcat(copyPathBuffer, currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->name);
    }
    else
    {
        strcpy(copyFileNameBuffer, currentMenu->directoryReader->files[currentMenu->item_ptr]->name);
        strcat(copyPathBuffer, currentMenu->directoryReader->files[currentMenu->item_ptr]->name);
    }
    currentOption = copyOption;
}


void FileManager::pasteItem()
{
    if (currentOption==copyOption || currentOption==cutOption)
    {
        char destination[maxPathLength];
        char fileName[100];
        strcpy(fileName, copyFileNameBuffer);
        strcpy(destination, currentMenu->directoryReader->currentPath);
        strcat(destination, "/");
        strcat(destination, fileName);

        // Check if file with same name exists, if yes throw error
        if (currentMenu->directoryReader->filesAndHiddenFiles.find(fileName)!=-1)
        {
            char msg[100];
            strcpy(msg, "A file with the name: ");
            strcat(msg, fileName);
            strcat(msg, " exists already");
            errorLoop(msg);
            return;
        }

        char* sourceFile = formatNameWithQuotes(copyPathBuffer);
        char* destinationFile = formatNameWithQuotes(destination);
        char* cmd = new char[6+strlen(sourceFile)+strlen(destinationFile)+21];
        strcpy(cmd, "cp -r ");
        strcat(cmd, sourceFile);
        strcat(cmd, " ");
        strcat(cmd, destinationFile);
        strcat(cmd, " > /dev/null 2>&1 &");
        system(cmd);
        delete[] cmd;

        // Wait for item to register in file system
        int itemLen = currentMenu->directoryReader->filesAndHiddenFiles.getLength();
        while (currentMenu->directoryReader->filesAndHiddenFiles.getLength()==itemLen)
        {
            currentMenu->directoryReader->read(".");
        }
        sortItems(currentMenu, currentMenu->itemSortMode);

        // Find newly pasted item and highlight it
        int newItemIdx;
        int fileListLen;
        if (currentMenu->showHidden)
        {
            newItemIdx = currentMenu->directoryReader->filesAndHiddenFiles.find(fileName);
            fileListLen = currentMenu->directoryReader->filesAndHiddenFiles.getLength();
        }
        else
        {
            newItemIdx = currentMenu->directoryReader->files.find(fileName);
            fileListLen = currentMenu->directoryReader->files.getLength();
        }

        // Highlight new item
        highlighter(1, 0);
        if (newItemIdx < currentMenu->nRows-1)
        {
            currentMenu->scrollOffset = 0;
            currentMenu->item_ptr = newItemIdx;
        }
        else if (fileListLen-newItemIdx < currentMenu->nRows-1)
        {
            currentMenu->scrollOffset = fileListLen - currentMenu->nRows;
            currentMenu->item_ptr = newItemIdx;
        }
        else
        {
            currentMenu->scrollOffset = newItemIdx;
            currentMenu->item_ptr = newItemIdx;
        }
        printMenuItems(currentMenu);
        highlighter(0, currentMenu->item_ptr-currentMenu->scrollOffset);
        
        // If cut option is on, then delete source
        if (currentOption==cutOption)
        {
            char* cmd = new char[6+strlen(sourceFile)+20];
            strcpy(cmd, "rm -r ");
            strcat(cmd, sourceFile);
            strcat(cmd, " ");
            strcat(cmd, " > /dev/null 2>&1 &");
            system(cmd);
            delete[] cmd;

            // Refresh the other menu (not current menu) in case that file is there
            menuEventLoop(KEY_STAB);
            itemLen = currentMenu->directoryReader->filesAndHiddenFiles.getLength();
            while (currentMenu->directoryReader->filesAndHiddenFiles.getLength()==itemLen)
            {
                currentMenu->directoryReader->read(".");
            }
            currentMenu->item_ptr = 0;
            currentMenu->scrollOffset = 0;
            sortItems(currentMenu, currentMenu->itemSortMode);
            printMenuItems(currentMenu);
            displayFileName(currentMenu);
            highlighter(0, 0);
            menuEventLoop(KEY_STAB);

            strcpy(copyPathBuffer, "\0");
            strcpy(copyFileNameBuffer, "\0");
            currentOption = none;
        }

        delete[] sourceFile;
        delete[] destinationFile;
    }

}


/**
 * Loop when help option is pressed
 */
void FileManager::helpLoop()
{
    // Display keybindings etc.
    WINDOW* helpWin = printBindings();


    bool inLoop = true;
    int keyboardInput;
    while (inLoop)
    {
        keyboardInput = getch();
        if (keyboardInput == KEY_ESCAPE)
        {
            clear();
            wclear(helpWin);
            untouchwin(helpWin);
            resizeFileManager();
            wrefresh(helpWin);
            delwin(helpWin);
            inLoop = false;
        }
        else if (keyboardInput==KEY_RESIZE)
        {
            resizeFileManager();
            wclear(helpWin);
            delwin(helpWin);
            helpWin = printBindings();
        }
    }    
}


WINDOW* FileManager::printBindings()
{
    char keys[19][20] = {
        "Enter",
        "Tab",
        "Function keys",
        "Ctrl+q",
        "c",
        "v",
        "x",
        "Ctrl+h",
        "Ctrl+n",
        "Ctrl+s",
        "Ctrl+u",
        "Ctrl+k",
        "Ctrl+r",
        "Ctrl+a",
        "Alt+left",
        "Shift+h",
        "Shift+i",
        "Delete",
        "Esc"       
    };

    char keybindings[19][35] = {
        "Open a folder/file",
        "Switch file managers",
        "Options at bottom",
        "Exit program",
        "Copy selection",
        "Paste selection",
        "Cut selection",
        "Show hidden items",
        "Sort by name",
        "Sort by size",
        "Sort by time modified",
        "Make new directory",
        "Rename selection",
        "Change file attributes",
        "Go back to prev page",
        "Show help page",
        "Show file info",
        "Delete selection",
        "Exit option pages"
    };

    char mouseOptions[15][15] = {
        "[.]",
        "<-",
        "Name",
        "Size",
        "Modify Time",
        "Help",
        "Info",
        "Copy",
        "Paste",
        "Cut",
        "Delete",
        "Mkdir",
        "Rename",
        "Attrib",
        "Quit"
    };

    char mousebindings[15][35] = {
        "Show hidden items",
        "Go back to prev page",
        "Sort by name",
        "Sort by size",
        "Sort by time modified",
        "Show help page",
        "Show file info",
        "Copy selection",
        "Paste selection",
        "Cut selection",
        "Delete selection",
        "Make new directory",
        "Rename selection",
        "Change file attributes",
        "Exit program",        
    };


    int nRows = LINES-4;
    int nCols = COLS-6;
    int starty =  (LINES-nRows) / 2;
    int startx = (COLS-nCols) / 2;
    WINDOW* helpWin = newwin(nRows, nCols, starty, startx);
    box(helpWin,0,0);
    wbkgd(helpWin, COLOR_PAIR(OPTIONS_WIN_COLOR));
    wattron(helpWin, COLOR_PAIR(OPTIONS_TITLE_COLOR) | A_BOLD);
    mvwprintw(helpWin, 0, nCols/2-4, "  Help  ");
    wattroff(helpWin, COLOR_PAIR(OPTIONS_TITLE_COLOR) | A_BOLD);

    wattron(helpWin, A_ITALIC);
    mvwprintw(helpWin, 1, nCols/2-7, "(esc to exit)");
    wattroff(helpWin, A_ITALIC);

    wattron(helpWin, A_BOLD | A_UNDERLINE);
    mvwprintw(helpWin, 1, nCols/2-25, "Keybindings");
    wattroff(helpWin, A_BOLD | A_UNDERLINE);
    
    wattron(helpWin, A_BOLD | A_UNDERLINE);
    mvwprintw(helpWin, 1, nCols/2+16, "Mouse bindings");
    wattroff(helpWin, A_BOLD | A_UNDERLINE);

    // Print all keybindings
    int row = 3;
    int numKeybindings = 19;
    for (int i=0; i<numKeybindings; ++i)
    {
        if (row < nRows-1)
        {
            mvwprintw(helpWin, row, nCols/2-42, "%s", keybindings[i]);
            mvwprintw(helpWin, row, nCols/2-15, "%s", keys[i]);
            row += 1;
        }        
    }

    row = 3;
    int numMousebindings = 15;
    for (int i=0; i<numMousebindings; ++i)
    {
        if (row < nRows-1)
        {
            mvwprintw(helpWin, row, nCols/2+8, "%s", mousebindings[i]);
            mvwprintw(helpWin, row, nCols/2+34, "%s", mouseOptions[i]);
            row += 1;
        }
    }
    
    wrefresh(helpWin);
    return helpWin;
}


/**
 * Delete a file/folder
 */
void FileManager::deleteLoop()
{
    // rm -r "filename"
    if (currentMenu->item_ptr==0)
    {
        errorLoop("Cannot operate on \"..\" folder");
        return;
    }

    // Are you sure you want to delete?
    int nRows = 9;
    int nCols = 50;
    int starty =  (LINES-nRows) / 2;
    int startx = (COLS-nCols) / 2;
    WINDOW* confirmWin = newwin(nRows, nCols, starty, startx);
    box(confirmWin,0,0);
    wbkgd(confirmWin, COLOR_PAIR(OPTIONS_WIN_COLOR));
    wattron(confirmWin, COLOR_PAIR(ERROR_COLOR) | A_BOLD);
    mvwprintw(confirmWin, 0, nCols/2-6, "  WARNING  ");
    wattroff(confirmWin, COLOR_PAIR(ERROR_COLOR) | A_BOLD);

    wattron(confirmWin, A_BOLD);
    mvwprintw(confirmWin, 2, nCols/2-12, "Are you sure you want to");
    mvwprintw(confirmWin, 3, nCols/2-12, " proceed with deletion?");
    wattroff(confirmWin, A_BOLD);

    mvwprintw(confirmWin, 5, nCols/2-20, "Press ESC to cancel or ENTER to proceed");

    wrefresh(confirmWin);
    bool inLoop = true;
    int keyboardInput;
    while (inLoop)
    {
        keyboardInput = getch();
        if (keyboardInput == KEY_ESCAPE)
        {
            clear();
            wclear(confirmWin);
            untouchwin(confirmWin);
            resizeFileManager();
            wrefresh(confirmWin);
            delwin(confirmWin);
            inLoop = false;
            return;
        }
        else if (keyboardInput==KEY_ENTER)
        {
            clear();
            wclear(confirmWin);
            untouchwin(confirmWin);
            resizeFileManager();
            wrefresh(confirmWin);
            delwin(confirmWin);
            inLoop = false;
        }
        else if (keyboardInput==KEY_RESIZE)
        {
            resizeFileManager();
            touchwin(confirmWin);
            wrefresh(confirmWin);
        }
    }

    char path[maxPathLength];
    strcpy(path, currentMenu->directoryReader->currentPath);
    strcat(path, "/");
    if (currentMenu->showHidden)
    {
        strcat(path, currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->name);
    }
    else
    {
        strcat(path, currentMenu->directoryReader->files[currentMenu->item_ptr]->name);
    }
    
    char* formattedPath = formatNameWithQuotes(path);
    char* cmd = new char[6+strlen(formattedPath)+22];
    strcpy(cmd, "rm -r ");
    strcat(cmd, formattedPath);
    strcat(cmd, " ");
    strcat(cmd, " > /dev/null 2>&1 &");
    system(cmd);
    delete[] formattedPath;
    delete[] cmd;

    // Wait for item to register in file system
    int itemLen = currentMenu->directoryReader->filesAndHiddenFiles.getLength();
    while (currentMenu->directoryReader->filesAndHiddenFiles.getLength()==itemLen)
    {
        currentMenu->directoryReader->read(".");
    }

    // Handle case of last item on menu
    int maxItemPtr;
    if (currentMenu->showHidden)
    {
        maxItemPtr = currentMenu->directoryReader->filesAndHiddenFiles.getLength();
    }
    else
    {
        maxItemPtr = currentMenu->directoryReader->files.getLength();
    }
    if (currentMenu->item_ptr==maxItemPtr)
    {
        currentMenu->item_ptr -= 1;
    }
    if (currentMenu->scrollOffset>0)
    {
        currentMenu->scrollOffset -= 1;
    }
    sortItems(currentMenu, currentMenu->itemSortMode);
    printMenuItems(currentMenu);
    displayFileName(currentMenu);
    highlighter(0, currentMenu->item_ptr-currentMenu->scrollOffset);
}


/**
 * Loop when info option is pressed
 */
void FileManager::infoLoop()
{
    int nRows = 11;
    int nCols = 50;
    int starty =  (LINES-nRows) / 2;
    int startx = (COLS-nCols) / 2;
    WINDOW* infoWin = newwin(nRows, nCols, starty, startx);
    box(infoWin, 0, 0);
    wbkgd(infoWin, COLOR_PAIR(OPTIONS_WIN_COLOR));
    wattron(infoWin, COLOR_PAIR(OPTIONS_TITLE_COLOR) | A_BOLD);
    mvwprintw(infoWin, 0, nCols/2-4, "  Info  ");
    wattroff(infoWin, COLOR_PAIR(OPTIONS_TITLE_COLOR) | A_BOLD);

    wattron(infoWin, A_ITALIC);
    mvwprintw(infoWin, 1, nCols/2-7, "(esc to exit)");
    wattroff(infoWin, A_ITALIC);

    char info[7][100];
    if (currentMenu->showHidden)
    {
        strcpy(info[0], currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->name);
        strcpy(info[1], currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->type);
        strcpy(info[2], currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->size);
        strcpy(info[3], currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->permissions);
        strcpy(info[4], currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->timeModified);
        strcpy(info[5], currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->timeAccessed);
        strcpy(info[6], currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->extension);
    }
    else
    {
        strcpy(info[0], currentMenu->directoryReader->files[currentMenu->item_ptr]->name);
        strcpy(info[1], currentMenu->directoryReader->files[currentMenu->item_ptr]->type);
        strcpy(info[2], currentMenu->directoryReader->files[currentMenu->item_ptr]->size);
        strcpy(info[3], currentMenu->directoryReader->files[currentMenu->item_ptr]->permissions);
        strcpy(info[4], currentMenu->directoryReader->files[currentMenu->item_ptr]->timeModified);
        strcpy(info[5], currentMenu->directoryReader->files[currentMenu->item_ptr]->timeAccessed);
        strcpy(info[6], currentMenu->directoryReader->files[currentMenu->item_ptr]->extension);
    }

    char fields[7][15] = {
        "Name:",
        "Type:",
        "Size:",
        "Permissions:",
        "Time Modified:",
        "Time Accessed:",
        "Extension:"
    };

    for (int i=0; i<7; ++i)
    {
        wattron(infoWin, A_BOLD);
        mvwprintw(infoWin, i+3, 1, "%s", fields[i]);
        wattroff(infoWin, A_BOLD);

        mvwprintw(infoWin, i+3, strlen(fields[i])+2, "%s", info[i]);
    }

    wrefresh(infoWin);

    bool inLoop = true;
    int keyboardInput;
    while (inLoop)
    {
        keyboardInput = getch();
        if (keyboardInput == KEY_ESCAPE)
        {
            clear();
            wclear(infoWin);
            untouchwin(infoWin);
            resizeFileManager();
            wrefresh(infoWin);
            delwin(infoWin);
            inLoop = false;
        }
        else if (keyboardInput==KEY_RESIZE)
        {
            resizeFileManager();
            touchwin(infoWin);
            wrefresh(infoWin);
        }
    }
}


/**
 * Loop when mkdir option is pressed
 */
void FileManager::mkdirLoop()
{
    FIELD* field[2];
    FORM* form;
    WINDOW* formWin;
    curs_set(1);
    
    // Initialize fields
    int nRows = 1;
    int nCols = 60;
    int starty =  (LINES-nRows) / 2;
    int startx = (COLS-nCols) / 2;
    field[0] = new_field(nRows, nCols, starty, startx, 0, 0);
    field[1] = NULL;
    field_opts_off(field[0], O_AUTOSKIP);
    
    set_field_fore(field[0], COLOR_PAIR(FORM_COLOR));
    set_field_back(field[0], COLOR_PAIR(FORM_COLOR));
    
    form = new_form(field);

    // Create window for form
    int nRowsWin = 7;
    int nColsWin = 65;
    int startyWin = (LINES-nRowsWin)/2;
    int startxWin = (COLS-nColsWin)/2;
    formWin = newwin(nRowsWin, nColsWin, startyWin, startxWin);
    box(formWin, 0, 0);
    wbkgd(formWin, COLOR_PAIR(OPTIONS_WIN_COLOR));

    wattron(formWin, COLOR_PAIR(OPTIONS_TITLE_COLOR) | A_BOLD);
    mvwprintw(formWin, 0, nColsWin/2-13, "  Create a new Directory  ");
    wattroff(formWin, COLOR_PAIR(OPTIONS_TITLE_COLOR) | A_BOLD);

    wattron(formWin, A_ITALIC);
    mvwprintw(formWin, nRowsWin-2, 16, "(esc to cancel, enter to create)");
    wattroff(formWin, A_ITALIC);


    // Set the form window and post it
    set_form_win(form, formWin);
    post_form(form);
    resizeFileManager();
    wrefresh(formWin);
    form_driver(form, (int)' ');
    form_driver(form, REQ_DEL_PREV);

    int keyboardInput;
    bool inLoop = true;
    int bufferLen = 0;
    int itemLen;
    int newItemIdx;
    int fileListLen;
    int result;
    while (inLoop)
    {
        keyboardInput = getch();
        switch (keyboardInput)
        {
        case KEY_LEFT:
			form_driver(form, REQ_PREV_CHAR);
			break;

        case KEY_RIGHT:
			form_driver(form, REQ_NEXT_CHAR);
			break;

        case KEY_BACKSPACE:
			form_driver(form, REQ_DEL_PREV);
            bufferLen -= 1;
			break;
        
        case KEY_DC:
			form_driver(form, REQ_DEL_CHAR);
            bufferLen -= 1;
			break;

        case KEY_ESCAPE:
            inLoop = false;
            break;

        case KEY_RESIZE:
            touchwin(formWin);
            unpost_form(form);
            post_form(form);
            resizeFileManager();
            form_driver(form, (int)' ');
            form_driver(form, REQ_DEL_PREV);
            wrefresh(formWin);
            break;

        case KEY_ENTER:
            form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_PREV_FIELD);
            char name[62];
            strcpy(name, trimFormWhitespaces(field_buffer(field[0], 0)));

            result = mkdir(name, 0777);
            if (result<0)
            {
                char msg[28+62];
                strcpy(msg, "Could not create directory ");
                strcat(msg, name);
                errorLoop(msg);
                touchwin(formWin);
                wrefresh(formWin);
                form_driver(form, (int)' ');
                form_driver(form, REQ_DEL_PREV);
            }

            else
            {
                inLoop = false;

                // Wait for item to register in file system
                itemLen = currentMenu->directoryReader->filesAndHiddenFiles.getLength();
                while (currentMenu->directoryReader->filesAndHiddenFiles.getLength()==itemLen)
                {
                    currentMenu->directoryReader->read(".");
                }
                sortItems(currentMenu, currentMenu->itemSortMode);
                // Find newly created directory and highlight it
                if (currentMenu->showHidden)
                {
                    newItemIdx = currentMenu->directoryReader->filesAndHiddenFiles.find(field_buffer(field[0], 0));
                    fileListLen = currentMenu->directoryReader->filesAndHiddenFiles.getLength();
                }
                else
                {
                    newItemIdx = currentMenu->directoryReader->files.find(field_buffer(field[0], 0));
                    fileListLen = currentMenu->directoryReader->files.getLength();
                }
                // Highlight new item
                highlighter(1, 0);
                if (newItemIdx < currentMenu->nRows-1)
                {
                    currentMenu->scrollOffset = 0;
                    currentMenu->item_ptr = newItemIdx;
                }
                else if (fileListLen-newItemIdx < currentMenu->nRows-1)
                {
                    currentMenu->scrollOffset = fileListLen - currentMenu->nRows;
                    currentMenu->item_ptr = newItemIdx;
                }
                else
                {
                    currentMenu->scrollOffset = newItemIdx;
                    currentMenu->item_ptr = newItemIdx;
                }
                printMenuItems(currentMenu);
                highlighter(0, currentMenu->item_ptr-currentMenu->scrollOffset);
            }
            
            break;
        
        default:
            if (bufferLen < nCols-1)
            {
                form_driver(form, keyboardInput);
                bufferLen += 1;
            }
            break;
        }
    }

    curs_set(0);
    unpost_form(form);
	free_form(form);
	free_field(field[0]);
	free_field(field[1]);
    resizeFileManager();
}


/**
 * Loop when rename option is pressed
 */
void FileManager::renameLoop()
{
    if (currentMenu->item_ptr==0)
    {
        errorLoop("Cannot operate on \"..\" folder");
        return;
    }

    FIELD* field[2];
    FORM* form;
    WINDOW* formWin;
    curs_set(1);
    
    // Initialize fields
    int nRows = 1;
    int nCols = 60;
    int starty =  (LINES-nRows) / 2;
    int startx = (COLS-nCols) / 2;
    field[0] = new_field(nRows, nCols, starty, startx, 0, 0);
    field[1] = NULL;
    field_opts_off(field[0], O_AUTOSKIP);
    
    set_field_fore(field[0], COLOR_PAIR(FORM_COLOR));
    set_field_back(field[0], COLOR_PAIR(FORM_COLOR));
    
    form = new_form(field);

    // Create window for form
    int nRowsWin = 7;
    int nColsWin = 65;
    int startyWin = (LINES-nRowsWin)/2;
    int startxWin = (COLS-nColsWin)/2;
    formWin = newwin(nRowsWin, nColsWin, startyWin, startxWin);
    box(formWin, 0, 0);
    wbkgd(formWin, COLOR_PAIR(OPTIONS_WIN_COLOR));

    wattron(formWin, COLOR_PAIR(OPTIONS_TITLE_COLOR) | A_BOLD);
    mvwprintw(formWin, 0, nColsWin/2-9, "  Rename a File  ");
    wattroff(formWin, COLOR_PAIR(OPTIONS_TITLE_COLOR) | A_BOLD);

    wattron(formWin, A_ITALIC);
    mvwprintw(formWin, nRowsWin-2, 16, "(esc to cancel, enter to rename)");
    wattroff(formWin, A_ITALIC);


    // Set the form window and post it
    set_form_win(form, formWin);
    post_form(form);
    resizeFileManager();
    wrefresh(formWin);
    form_driver(form, (int)' ');
    form_driver(form, REQ_DEL_PREV);

    int keyboardInput;
    bool inLoop = true;
    int bufferLen = 0;
    int newItemIdx;
    int fileListLen;
    int result;
    while (inLoop)
    {
        keyboardInput = getch();
        switch (keyboardInput)
        {
        case KEY_LEFT:
			form_driver(form, REQ_PREV_CHAR);
			break;

        case KEY_RIGHT:
			form_driver(form, REQ_NEXT_CHAR);
			break;

        case KEY_BACKSPACE:
			form_driver(form, REQ_DEL_PREV);
            bufferLen -= 1;
			break;
        
        case KEY_DC:
			form_driver(form, REQ_DEL_CHAR);
            bufferLen -= 1;
			break;

        case KEY_ESCAPE:
            inLoop = false;
            break;

        case KEY_RESIZE:
            touchwin(formWin);
            unpost_form(form);
            post_form(form);
            resizeFileManager();
            form_driver(form, (int)' ');
            form_driver(form, REQ_DEL_PREV);
            wrefresh(formWin);
            break;

        case KEY_ENTER:
            form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_PREV_FIELD);
            char oldname[100];
            char newname[100];
            if (currentMenu->showHidden)
            {
                strcpy(oldname, currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->name);
            }
            else
            {
                strcpy(oldname, currentMenu->directoryReader->files[currentMenu->item_ptr]->name);
            }
            strcpy(newname, trimFormWhitespaces(field_buffer(field[0], 0)));

            // Check if there exists a file/folder with the same name
            if (currentMenu->directoryReader->filesAndHiddenFiles.find(newname)!=-1)
            {
                char msg[28+62];
                strcpy(msg, "Could not rename file to ");
                strcat(msg, newname);
                errorLoop(msg);
                touchwin(formWin);
                wrefresh(formWin);
                form_driver(form, (int)' ');
                form_driver(form, REQ_DEL_PREV);
                continue;
            }

            // Rename and display error if unsuccessful
            result = rename(oldname, newname);
            if (result<0)
            {
                char msg[28+62];
                strcpy(msg, "Could not rename file to ");
                strcat(msg, newname);
                errorLoop(msg);
                touchwin(formWin);
                wrefresh(formWin);
                form_driver(form, (int)' ');
                form_driver(form, REQ_DEL_PREV);
            }
            else
            {
                inLoop = false;
                currentMenu->directoryReader->read(".");
                sortItems(currentMenu, currentMenu->itemSortMode);

                // Find newly renamed directory and highlight it
                if (currentMenu->showHidden)
                {
                    newItemIdx = currentMenu->directoryReader->filesAndHiddenFiles.find(field_buffer(field[0], 0));
                    fileListLen = currentMenu->directoryReader->filesAndHiddenFiles.getLength();
                }
                else
                {
                    newItemIdx = currentMenu->directoryReader->files.find(field_buffer(field[0], 0));
                    fileListLen = currentMenu->directoryReader->files.getLength();
                }
                // Highlight new item
                highlighter(1, 0);
                if (newItemIdx < currentMenu->nRows-1)
                {
                    currentMenu->scrollOffset = 0;
                    currentMenu->item_ptr = newItemIdx;
                }
                else if (fileListLen-newItemIdx < currentMenu->nRows-1)
                {
                    currentMenu->scrollOffset = fileListLen - currentMenu->nRows;
                    currentMenu->item_ptr = newItemIdx;
                }
                else
                {
                    currentMenu->scrollOffset = newItemIdx;
                    currentMenu->item_ptr = newItemIdx;
                }
                printMenuItems(currentMenu);
                highlighter(0, currentMenu->item_ptr-currentMenu->scrollOffset);
            }
            break;

        default:
            if (bufferLen < nCols-1)
            {
                form_driver(form, keyboardInput);
                bufferLen += 1;
            }
            break;
        }
    }

    curs_set(0);
    unpost_form(form);
	free_form(form);
	free_field(field[0]);
	free_field(field[1]);
    resizeFileManager();
}


void FileManager::errorLoop(char* msg)
{
    int nRows = 7;
    int nCols = strlen(msg)+3;
    int starty =  (LINES-nRows) / 2;
    int startx = (COLS-nCols) / 2;
    WINDOW* errorWin = newwin(nRows, nCols, starty, startx);
    box(errorWin,0,0);
    wbkgd(errorWin, COLOR_PAIR(OPTIONS_WIN_COLOR));
    wattron(errorWin, COLOR_PAIR(ERROR_COLOR) | A_BOLD);
    mvwprintw(errorWin, 0, nCols/2-4, "  ERROR  ");
    wattroff(errorWin, COLOR_PAIR(ERROR_COLOR) | A_BOLD);

    wattron(errorWin, A_ITALIC);
    mvwprintw(errorWin, 1, nCols/2-7, "(esc to exit)");
    wattroff(errorWin, A_ITALIC);

    wattron(errorWin, A_BOLD);
    mvwprintw(errorWin, 3, 1, "%s", msg);
    wattroff(errorWin, A_BOLD);

    wrefresh(errorWin);

    bool inLoop = true;
    int keyboardInput;
    while (inLoop)
    {
        keyboardInput = getch();
        if (keyboardInput == KEY_ESCAPE)
        {
            clear();
            wclear(errorWin);
            untouchwin(errorWin);
            resizeFileManager();
            wrefresh(errorWin);
            delwin(errorWin);
            inLoop = false;
        }
        else if (keyboardInput==KEY_RESIZE)
        {
            resizeFileManager();
            touchwin(errorWin);
            wrefresh(errorWin);
        }
    }
}


void FileManager::attributeLoop()
{
    FIELD* field[2];
    FORM* form;
    WINDOW* formWin;
    curs_set(1);

    // Initialize fields
    int nRows = 1;
    int nCols = 10;
    int starty =  (LINES-nRows) / 2;
    int startx = (COLS-nCols) / 2;
    field[0] = new_field(nRows, nCols, starty+3, startx, 0, 0);
    field[1] = NULL;
    field_opts_off(field[0], O_AUTOSKIP);
    
    set_field_fore(field[0], COLOR_PAIR(FORM_COLOR));
    set_field_back(field[0], COLOR_PAIR(FORM_COLOR));
    
    form = new_form(field);

    // Create window for form
    int nRowsWin = 15;
    int nColsWin = 40;
    int startyWin = (LINES-nRowsWin)/2;
    int startxWin = (COLS-nColsWin)/2;
    formWin = newwin(nRowsWin, nColsWin, startyWin, startxWin);
    box(formWin, 0, 0);
    wbkgd(formWin, COLOR_PAIR(OPTIONS_WIN_COLOR));

    wattron(formWin, COLOR_PAIR(OPTIONS_TITLE_COLOR) | A_BOLD);
    mvwprintw(formWin, 0, nColsWin/2-13, "  Change file attributes  ");
    wattroff(formWin, COLOR_PAIR(OPTIONS_TITLE_COLOR) | A_BOLD);

    mvwprintw(formWin, 2, nColsWin/2-13, "The attributes are in the");
    mvwprintw(formWin, 3, nColsWin/2-9, " following order:");

    wattron(formWin, A_BOLD | A_UNDERLINE);
    mvwprintw(formWin, 5, nColsWin/2-11, "Owner   Group   Others");
    mvwprintw(formWin, 6, nColsWin/2-11, "Read   Write   Execute");
    wattroff(formWin, A_BOLD | A_UNDERLINE);

    mvwprintw(formWin, 7, nColsWin/2-14, "r: read  w: write  x: execute");
    mvwprintw(formWin, 8, nColsWin/2-8, "-: no permission");

    wattron(formWin, A_ITALIC);
    mvwprintw(formWin, 12, nColsWin/2-15, "ESC to cancel ENTER to proceed");
    wattroff(formWin, A_ITALIC);

    // Set the form window and post it
    set_form_win(form, formWin);
    post_form(form);
    resizeFileManager();
    wrefresh(formWin);
    form_driver(form, (int)' ');
    form_driver(form, REQ_DEL_PREV);

    // Fill the buffer with current attributes
    for (int i=0; i<9; ++i)
    {
        if (currentMenu->showHidden)
        {
            form_driver(form, (int)currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->permissions[i]);
        }
        else
        {
            form_driver(form, (int)currentMenu->directoryReader->files[currentMenu->item_ptr]->permissions[i]);
        }
    }

    int keyboardInput;
    bool inLoop = true;
    int bufferLen = 9;
    int result;
    int permission_bits = 0;
    while (inLoop)
    {
        keyboardInput = getch();
        switch (keyboardInput)
        {
        case KEY_LEFT:
			form_driver(form, REQ_PREV_CHAR);
			break;

        case KEY_RIGHT:
			form_driver(form, REQ_NEXT_CHAR);
			break;

        case KEY_BACKSPACE:
			form_driver(form, REQ_DEL_PREV);
            bufferLen -= 1;
			break;
        
        case KEY_DC:
			form_driver(form, REQ_DEL_CHAR);
            bufferLen -= 1;
			break;

        case KEY_ESCAPE:
            inLoop = false;
            break;

        case KEY_RESIZE:
            touchwin(formWin);
            unpost_form(form);
            post_form(form);
            resizeFileManager();
            form_driver(form, (int)' ');
            form_driver(form, REQ_DEL_PREV);
            wrefresh(formWin);
            // Fill the buffer with current attributes
            for (int i=0; i<9; ++i)
            {
                if (currentMenu->showHidden)
                {
                    form_driver(form, (int)currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->permissions[i]);
                }
                else
                {
                    form_driver(form, (int)currentMenu->directoryReader->files[currentMenu->item_ptr]->permissions[i]);
                }
                bufferLen += 1;
            }
            break;

        case KEY_ENTER:
            form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_PREV_FIELD);
            char permissions[10];
            strcpy(permissions, trimFormWhitespaces(field_buffer(field[0], 0)));

            // Validation check
            for (int i=0; i<9; ++i)
            {
                if (permissions[i]!='r' && permissions[i]!='w' && permissions[i]!='x' && permissions[i]!='-')
                {
                    errorLoop("Invalid character input");
                    touchwin(formWin);
                    wrefresh(formWin);
                    form_driver(form, (int)' ');
                    form_driver(form, REQ_DEL_PREV);
                    break;
                }
            }

            for (int i=0; i<3; ++i)
            {
                if (permissions[i]=='r')
                {
                    permission_bits |= S_IRUSR;
                }
                else if (permissions[i]=='w')
                {
                    permission_bits |= S_IWUSR;
                }
                else if (permissions[i]=='x')
                {
                    permission_bits |= S_IXUSR;
                }
            }
            for (int i=3; i<6; ++i)
            {
                if (permissions[i]=='r')
                {
                    permission_bits |= S_IRGRP;
                }
                else if (permissions[i]=='w')
                {
                    permission_bits |= S_IWGRP;
                }
                else if (permissions[i]=='x')
                {
                    permission_bits |= S_IXGRP;
                }
            }
            for (int i=6; i<9; ++i)
            {
                if (permissions[i]=='r')
                {
                    permission_bits |= S_IROTH;
                }
                else if (permissions[i]=='w')
                {
                    permission_bits |= S_IWOTH;
                }
                else if (permissions[i]=='x')
                {
                    permission_bits |= S_IXOTH;
                }
            }

            if (currentMenu->showHidden)
            {
                result = chmod(currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->name, permission_bits);
            }
            else
            {
                result = chmod(currentMenu->directoryReader->files[currentMenu->item_ptr]->name, permission_bits);
            }

            if (result < 0)
            {
                char msg[28+62];
                strcpy(msg, "Could not change permissions");
                errorLoop(msg);
                touchwin(formWin);
                wrefresh(formWin);
                form_driver(form, (int)' ');
                form_driver(form, REQ_DEL_PREV);
            }
            else
            {
                inLoop = false;
                currentMenu->directoryReader->read(".");
                sortItems(currentMenu, currentMenu->itemSortMode);
                highlighter(0, currentMenu->item_ptr);
            }
            
            break;
        
        default:
            if (bufferLen < nCols-1)
            {
                form_driver(form, keyboardInput);
                bufferLen += 1;
            }
            break;
        }
    }

    curs_set(0);
    unpost_form(form);
	free_form(form);
	free_field(field[0]);
	free_field(field[1]);
    resizeFileManager();
}


char* FileManager::trimFormWhitespaces(char* str)
{
    char *end;

	// Trim leading space
	while(isspace(*str))
		str++;

    // All spaces?
	if(*str == 0) 
		return str;

	// Trim trailing space
	end = str + strnlen(str, 128) - 1;

	while(end > str && isspace(*end))
		end--;

	// Write new null terminator
	*(end+1) = '\0';

	return str;
}