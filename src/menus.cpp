#include "menus.h"
#include "directory.h"
#include "keybindings.h"
#include "colors.h"
#include <ncurses.h>
#include <menu.h>
#include <math.h>
#include <string.h>
#include <unistd.h>



/**
 * Constructor calls the panels constructor which
 * sets up Ncurses and draws the panels.
 */
Menus::Menus(char* rootDir) : Panels(), currentMenu(&leftMenu)
{
    leftMenu.itemSortMode = byName;
    rightMenu.itemSortMode = byName;
    leftMenu.directoryReader = new DirectoryReader(rootDir);
    rightMenu.directoryReader = new DirectoryReader(rootDir);

    // Initialize which menu corresponds to which panel
    leftMenu.panelWindow = &leftPanel;
    rightMenu.panelWindow = &rightPanel;

    // Sort items by name (default)
    sortItems(&leftMenu, byName);
    sortItems(&rightMenu, byName);

    // Initialize hidden files option
    leftMenu.showHidden = false;
    rightMenu.showHidden = false;

    // Initialize scroll offset
    leftMenu.scrollOffset = 0;
    rightMenu.scrollOffset = 0;

    // Initialize menu to point at first item
    leftMenu.item_ptr = 0;
    rightMenu.item_ptr = 0;
    
    createMenus();

    // Initialize interfacing with the mouse
    mousemask(ALL_MOUSE_EVENTS, NULL);
}


Menus::~Menus()
{
    delwin(leftMenu.window);
    delwin(rightMenu.window);
    delwin(leftMenu.nameDisplayWindow);
    delwin(rightMenu.nameDisplayWindow);
    delwin(leftMenu.pathDisplayWindow);
    delwin(rightMenu.pathDisplayWindow);

    delete leftMenu.directoryReader;
    delete rightMenu.directoryReader;
}


/**
 * Draws menus in terminal. Takes care of finding terminal size
 */
void Menus::createMenus()
{
    // Initialize menuWindow struct members
    // Rows and cols
    int nRows = leftPanel.nRows - 5;
    int nCols = leftPanel.nCols - 2;
    leftMenu.nRows = nRows;
    leftMenu.nCols = nCols;
    rightMenu.nRows = nRows;
    rightMenu.nCols = nCols;

    // Top left corner coordinates w.r.t panel windows
    leftMenu.topLeftCornerY = 2;
    leftMenu.topLeftCornerX = 1;
    rightMenu.topLeftCornerY = 2;
    rightMenu.topLeftCornerX = 1;

    // Create two new windows
    leftMenu.window = derwin(leftPanel.window, nRows, nCols, leftMenu.topLeftCornerY, leftMenu.topLeftCornerX);
    rightMenu.window = derwin(rightPanel.window, nRows, nCols, rightMenu.topLeftCornerY, rightMenu.topLeftCornerX);

    // Create window for displaying names of files
    leftMenu.nameDisplayWindow = derwin(leftPanel.window, 1, nCols, leftPanel.nRows-2, 1);
    rightMenu.nameDisplayWindow = derwin(rightPanel.window, 1, nCols, rightPanel.nRows-2, 1);

    // Create window for displaying path
    leftMenu.pathDisplayWindow = newwin(1, nCols, 0, 0);
    rightMenu.pathDisplayWindow = newwin(1, nCols, 0, nCols+2);
    wbkgd(leftMenu.pathDisplayWindow, COLOR_PAIR(BACKGROUND_COLOR));   
    wbkgd(rightMenu.pathDisplayWindow, COLOR_PAIR(BACKGROUND_COLOR));   

    // Initialize colPos struct parameters (where each column starts X-coordinates)
    leftMenu.colPos_1 = 0;
    leftMenu.colPos_2 = nCols-22;
    leftMenu.colPos_3 = nCols-14;
    
    rightMenu.colPos_1 = 0;
    rightMenu.colPos_2 = nCols-22;
    rightMenu.colPos_3 = nCols-14;

    displayFileName(&leftMenu);
    displayFileName(&rightMenu);

    displayPath(&leftMenu);
    displayPath(&rightMenu);

    printMenuItems(&leftMenu);
    printMenuItems(&rightMenu);

    showSortMode(&leftMenu);
    showSortMode(&rightMenu);

    // While resizing the terminal, the item pointer stays within the window
    if (currentMenu->item_ptr-currentMenu->scrollOffset >= currentMenu->nRows-1)
    {
        currentMenu->item_ptr = currentMenu->nRows-1 + currentMenu->scrollOffset;
    }
    highlighter(0, currentMenu->item_ptr-currentMenu->scrollOffset);
}


void Menus::printMenuItems(MenuWindow* menuWindow)
{
    // Clear the menu and re-draw the column lines (at the end)
    wclear(menuWindow->window);    

    int scrollOffset = menuWindow->scrollOffset;
    int offsetCount = 0;
    int filledRows = 0;
    
    // Print items on menu
    int limit;
    if (menuWindow->showHidden)
    {
        limit = std::min(menuWindow->nRows, menuWindow->directoryReader->filesAndHiddenFiles.getLength());
    }
    else
    {
        limit = std::min(menuWindow->nRows, menuWindow->directoryReader->files.getLength());
    }

    // Loop through all printable items and put them on the screen taking into account the scroll offset
    for (int i=0; i<limit+scrollOffset; ++i)
    {
        if (offsetCount < scrollOffset)
        {
            offsetCount += 1;
            continue;
        }

        File* file;
        char size[8];
        char timeModified[15];
        if (menuWindow->showHidden)
        {
            file = menuWindow->directoryReader->filesAndHiddenFiles[i];
            strcpy(size, menuWindow->directoryReader->filesAndHiddenFiles[i]->size);
            strcpy(timeModified, menuWindow->directoryReader->filesAndHiddenFiles[i]->timeModified);

        }
        else
        {
            file = menuWindow->directoryReader->files[i];
            strcpy(size, menuWindow->directoryReader->files[i]->size);
            strcpy(timeModified, menuWindow->directoryReader->files[i]->timeModified);
        }

        // Check if it is UP-DIR
        if (i==0)
        {
           strcpy(size, "UP--DIR");
        }

        // Check name len
        int nameMaxLen = menuWindow->colPos_2 - 2;
        char* name;

        // Could be negative due to terminal resizing
        if (nameMaxLen<=0)
        {
            name = new char[1];
            name[0] = '\0';
        }
        else
        {
            name = getFormattedItemName(file, nameMaxLen);
        }

        mvwprintw(menuWindow->window, filledRows, menuWindow->colPos_1, "%.*s", (menuWindow->colPos_2-1), name);
        mvwprintw(menuWindow->window, filledRows, menuWindow->colPos_2, "%7s", size);
        mvwprintw(menuWindow->window, filledRows, menuWindow->colPos_3, "%s", timeModified);
        filledRows += 1;
        delete[] name;
    }

    drawCols();
    wrefresh(menuWindow->window);
}


/**
 * Check if name is longer than column width and split
 * name into two halves so we can see extension of the file
 */
char* Menus::getFormattedItemName(File* file, int maxLen)
{
    int nameLen = strlen(file->name);
    char* name = new char[maxLen+1];

    if (nameLen > maxLen)
    {
        strncpy(name+1, file->name, (int)(maxLen/2));
        strcpy(name+maxLen/2+1, file->name + (nameLen-maxLen/2));
        name[(int)(maxLen/2)] = '~';
    }
    else
    {
        strcpy(name+1, file->name);
    }

    // Check if it's directory or not
    if (strcmp(file->type, "Directory") == 0)
    {
        name[0] = '/';
    }
    else
    {
        name[0] = ' ';
    }

    return name;
}


/**
 * Main loop that keeps menus running 
 */
void Menus::menuEventLoop(int input)
{
    switch (input)
    {

    /**
     * UP key pressed 
     */
    case KEY_UP:
    {
        // We can scroll without changing offset
        if ((currentMenu->item_ptr != 0) && (currentMenu->item_ptr-currentMenu->scrollOffset > 0))
        {
            // Remove previous highlighted line
            int row = currentMenu->item_ptr - currentMenu->scrollOffset;
            highlighter(1, row);
            highlighter(0, row-1);
            currentMenu->item_ptr -= 1;
            displayFileName(currentMenu);
        }
        // We can scroll only if we change the offset
        else
        {
            // Check if offset can decrease
            if (currentMenu->scrollOffset > 0)
            {
                currentMenu->scrollOffset -= 1;
                currentMenu->item_ptr -= 1;
                printMenuItems(currentMenu);

                int row = currentMenu->item_ptr - currentMenu->scrollOffset;
                highlighter(0, row);
                displayFileName(currentMenu);
            }
        }
        break;
    }

    /**
     * DOWN key pressed 
     */
    case KEY_DOWN:
    {
        int itemLen;
        if (currentMenu->showHidden)
        {
            itemLen = currentMenu->directoryReader->filesAndHiddenFiles.getLength();
        }
        else
        {
            itemLen = currentMenu->directoryReader->files.getLength();
        }

        // We can scroll down without changing offset
        // Check whether item_ptr-scrollOffset < nRows-1 (highlighted bar is in between top and bottom)
        // Check whether item_ptr < itemLen (less files than rows)
        if ((currentMenu->item_ptr-currentMenu->scrollOffset < currentMenu->nRows-1) && (currentMenu->item_ptr < itemLen-1))
        {
            // Remove previous highlighted line
            int row = currentMenu->item_ptr - currentMenu->scrollOffset;
            highlighter(1, row);
            highlighter(0, row+1);
            currentMenu->item_ptr += 1;
            displayFileName(currentMenu);
        }
        // We can scroll only if we change the offset
        else
        {
            // Check if offset can increase
            if (currentMenu->scrollOffset < itemLen - currentMenu->nRows)
            {
                currentMenu->scrollOffset += 1;
                currentMenu->item_ptr += 1;
                printMenuItems(currentMenu);

                int row = currentMenu->item_ptr - currentMenu->scrollOffset;
                highlighter(0, row);
                displayFileName(currentMenu);
            }

        }
        break;
    }

    /**
     * TAB key pressed, switch menus
     */
    case KEY_STAB:
    {
        int row = currentMenu->item_ptr - currentMenu->scrollOffset;
        highlighter(1, row);
        
        // Change menus
        currentMenu = (currentMenu == &leftMenu) ? &rightMenu : &leftMenu;
        row = currentMenu->item_ptr - currentMenu->scrollOffset;
        highlighter(0, row);
        chdir(currentMenu->directoryReader->currentPath);
        break;
    }

    /**
     * ENTER key has been pressed, go into directories
     */
    case KEY_ENTER:
    {
        if (currentMenu->showHidden)
        {
            // Enter into directories
            if (strcmp(currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->type, "Directory") == 0)
            {
                int item_ptr_temp = 0;
                int scrollOffsetTemp = 0;
                if (currentMenu->item_ptr==0)
                {
                    if (currentMenu->history.getLength()>0)
                    {
                        int idx = currentMenu->history.getLength()-1;
                        item_ptr_temp = currentMenu->history[idx]->getItem_ptr();
                        scrollOffsetTemp = currentMenu->history[idx]->getScrollOffset();
                        currentMenu->history.del(idx);
                    }
                }
                else
                {
                    currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->setScrollOffset(currentMenu->scrollOffset);
                    currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->setItem_ptr(currentMenu->item_ptr);
                    File* dir = new Directory(*currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]);
                    currentMenu->history.append(dir);
                }

                currentMenu->directoryReader->read(currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->name);

                // Sort the newly read files
                sortItems(currentMenu, currentMenu->itemSortMode);
                currentMenu->item_ptr = item_ptr_temp;
                currentMenu->scrollOffset = scrollOffsetTemp;
                printMenuItems(currentMenu);
                highlighter(0, item_ptr_temp-scrollOffsetTemp);
                displayPath(currentMenu);
                displayFileName(currentMenu);
            }

            // Display files based on their extensions
            else
            {
                openFile();
            }
        }
        else
        {
            // Enter into directories
            if (strcmp(currentMenu->directoryReader->files[currentMenu->item_ptr]->type, "Directory") == 0)
            {
                int item_ptr_temp = 0;
                int scrollOffsetTemp = 0;
                if (currentMenu->item_ptr==0)
                {
                    if (currentMenu->history.getLength()>0)
                    {
                        int idx = currentMenu->history.getLength()-1;
                        item_ptr_temp = currentMenu->history[idx]->getItem_ptr();
                        scrollOffsetTemp = currentMenu->history[idx]->getScrollOffset();
                        currentMenu->history.del(idx);
                    }
                }
                else
                {
                    currentMenu->directoryReader->files[currentMenu->item_ptr]->setScrollOffset(currentMenu->scrollOffset);
                    currentMenu->directoryReader->files[currentMenu->item_ptr]->setItem_ptr(currentMenu->item_ptr);
                    File* dir = new Directory(*currentMenu->directoryReader->files[currentMenu->item_ptr]);
                    currentMenu->history.append(dir);
                }

                currentMenu->directoryReader->read(currentMenu->directoryReader->files[currentMenu->item_ptr]->name);

                // Sort the newly read files
                sortItems(currentMenu, currentMenu->itemSortMode);
                currentMenu->item_ptr = item_ptr_temp;
                currentMenu->scrollOffset = scrollOffsetTemp;
                printMenuItems(currentMenu);
                highlighter(0, item_ptr_temp-scrollOffsetTemp);
                displayPath(currentMenu);
                displayFileName(currentMenu);
            }

            // Display files based on their extensions
            else
            {
                openFile();
            }
            
        }            
        break;
    }

    /**
     * Sort items alphabetically
     */
    case SORT_BY_NAME:
    {
        // Clear history which will be messed up otherwise
        currentMenu->history.clear();

        sortItems(currentMenu, byName);
        currentMenu->item_ptr = 0;
        currentMenu->scrollOffset = 0;
        printMenuItems(currentMenu);
        displayFileName(currentMenu);
        highlighter(0, 0);
        break;
    }

    /**
     * Sort items by their size
     */
    case SORT_BY_SIZE:
    {
        // Clear history which will be messed up otherwise
        currentMenu->history.clear();

        sortItems(currentMenu, bySize);
        currentMenu->item_ptr = 0;
        currentMenu->scrollOffset = 0;
        printMenuItems(currentMenu);
        displayFileName(currentMenu);
        highlighter(0, 0);
        break;
    }
    
    /**
     * Sort items by their size
     */
    case SORT_BY_TIME_MODIFIED:
    {
        // Clear history which will be messed up otherwise
        currentMenu->history.clear();

        sortItems(currentMenu, byTimeModified);
        currentMenu->item_ptr = 0;
        currentMenu->scrollOffset = 0;
        printMenuItems(currentMenu);
        displayFileName(currentMenu);
        highlighter(0, 0);
        break;
    }

    /**
     * Change hidden file mode for current menu
     */
    case KEY_HIDDEN:
    {
        // Clear history which will be messed up otherwise
        currentMenu->history.clear();
        currentMenu->showHidden = (currentMenu->showHidden==true) ? false : true;
        currentMenu->item_ptr = 0;
        currentMenu->scrollOffset = 0;
        printMenuItems(currentMenu);
        highlighter(0, 0);
        displayFileName(currentMenu);
        break;
    }

    /**
     * Takes you back to where you were before
     */
    case KEY_BACK:
    {
        if (currentMenu->history.getLength()>0)
        {
            int idx = currentMenu->history.getLength()-1;
            currentMenu->item_ptr = currentMenu->history[idx]->getItem_ptr();
            currentMenu->scrollOffset = currentMenu->history[idx]->getScrollOffset();
            currentMenu->history.del(idx);

            if (currentMenu->showHidden)
            {
                currentMenu->directoryReader->read(currentMenu->directoryReader->filesAndHiddenFiles[0]->name);
            }
            else
            {
                currentMenu->directoryReader->read(currentMenu->directoryReader->files[0]->name);
            }

            // Sort the newly read files
            sortItems(currentMenu, currentMenu->itemSortMode);
            printMenuItems(currentMenu);
            highlighter(0, currentMenu->item_ptr-currentMenu->scrollOffset);
            displayPath(currentMenu);
            displayFileName(currentMenu);
        }
        break;
    }

    default:
        break;
    }
    wrefresh(currentMenu->window);
    
}


void Menus::highlighter(int mode, int row)
{
    // If mode is highlight
    if (mode==0)
    {
        mvwaddch(currentMenu->window, row, currentMenu->colPos_2-1, ' ');
        mvwaddch(currentMenu->window, row, currentMenu->colPos_3-1, ' ');
        mvwchgat(currentMenu->window, row, 0, -1, A_BOLD, 3, NULL);
    }
    // Else mode is unhighlighting
    else if (mode==1)
    {
        mvwchgat(currentMenu->window, row, 0, -1, A_NORMAL, 1, NULL);
        drawCols();
    }
    wrefresh(currentMenu->window);
}


void Menus::openFile()
{
    char* name;
    if (currentMenu->showHidden)
    {
        name = formatNameWithQuotes(currentMenu->directoryReader->filesAndHiddenFiles[currentMenu->item_ptr]->name);
    }
    else
    {
        name = formatNameWithQuotes(currentMenu->directoryReader->files[currentMenu->item_ptr]->name);
    }

    char* cmd = new char[9+strlen(name)+20];
    strcpy(cmd, "xdg-open ");
    strcat(cmd, name);
    strcat(cmd, " > /dev/null 2>&1 &");
    system(cmd);
    delete[] name;
    delete[] cmd;
}


char* Menus::formatNameWithQuotes(char* name)
{
    char* newName = new char[strlen(name)+2+1];
    strcpy(newName+1, name);
    newName[0] = '"';
    newName[strlen(name)+1] = '"';
    newName[strlen(name)+2] = '\0';
    return newName;
}


void Menus::displayFileName(MenuWindow* menuWindow)
{
    wclear(menuWindow->nameDisplayWindow);
    
    // Check if we have to display the UP--DIR
    if (menuWindow->item_ptr==0)
    {
        wprintw(menuWindow->nameDisplayWindow, "UP--DIR");
        wrefresh(menuWindow->nameDisplayWindow);
        return;
    }

    File* file;
    if (menuWindow->showHidden)
    {
        file = menuWindow->directoryReader->filesAndHiddenFiles[menuWindow->item_ptr];
    }
    else
    {
        file = menuWindow->directoryReader->files[menuWindow->item_ptr];
    }

    int nameMaxLen = menuWindow->nCols-1;
    char* name = getFormattedItemName(file, nameMaxLen);
    wprintw(menuWindow->nameDisplayWindow, "%s", name);

    wrefresh(menuWindow->nameDisplayWindow);
    delete[] name;
}


void Menus::displayPath(MenuWindow* menuWindow)
{
    char* path = formatPath(menuWindow->directoryReader->currentPath);

    wclear(menuWindow->pathDisplayWindow);
    wprintw(menuWindow->pathDisplayWindow, "%s", path);
    mvwchgat(menuWindow->pathDisplayWindow, 0, 0, strlen(path), A_NORMAL, 4, NULL);
    wrefresh(menuWindow->pathDisplayWindow);
    delete[] path;
}


char* Menus::formatPath(char* path)
{
    int pathLen = strlen(path);
    int maxPathLen = currentMenu->nCols;
    char* newPath = new char[maxPathLen+1];
    
    if (pathLen > maxPathLen)
    {
        strncpy(newPath, path, (int)(maxPathLen/2));
        strcpy(newPath+maxPathLen/2+1, path + (pathLen-maxPathLen/2));
        newPath[(int)(maxPathLen/2)] = '~';
    }
    else
    {
        strcpy(newPath, path);
    }

    return newPath;
}


void Menus::showSortMode(MenuWindow* menuWindow)
{
    wattron(menuWindow->panelWindow->window, COLOR_PAIR(SORT_MODE_COLOR) | A_BOLD);

    if (menuWindow->itemSortMode==byName)
    {
        mvwprintw(menuWindow->panelWindow->window, 1, 1, "^n");
    }
    else if (menuWindow->itemSortMode==bySize)
    {
        mvwprintw(menuWindow->panelWindow->window, 1, 1, "^s");
    }
    else if (menuWindow->itemSortMode==byTimeModified)
    {
        mvwprintw(menuWindow->panelWindow->window, 1, 1, "^t");
    }
    
    wattroff(menuWindow->panelWindow->window, COLOR_PAIR(SORT_MODE_COLOR) | A_BOLD);
    wrefresh(menuWindow->panelWindow->window);
}


void Menus::sortItems(MenuWindow* menuWindow, sortMode mode)
{
    if (mode==byName)
    {
        menuWindow->directoryReader->files.sortByName();
        menuWindow->directoryReader->filesAndHiddenFiles.sortByName();
        menuWindow->itemSortMode = byName;
    }
    else if (mode==bySize)
    {
        menuWindow->directoryReader->files.sortBySize();
        menuWindow->directoryReader->filesAndHiddenFiles.sortBySize();
        menuWindow->itemSortMode = bySize;
    }
    else if (mode==byTimeModified)
    {
        menuWindow->directoryReader->files.sortByTimeModified();
        menuWindow->directoryReader->filesAndHiddenFiles.sortByTimeModified();
        menuWindow->itemSortMode = byTimeModified;
    }
    showSortMode(menuWindow);
}


void Menus::resizeMenus()
{
    delwin(leftMenu.window);
    delwin(rightMenu.window);
    delwin(leftMenu.nameDisplayWindow);
    delwin(rightMenu.nameDisplayWindow);
    delwin(leftMenu.pathDisplayWindow);
    delwin(rightMenu.pathDisplayWindow);
    createMenus();
}