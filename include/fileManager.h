#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "menus.h"

#define maxPathLength 300


class FileManager : protected Menus
{
private:
    void resizeFileManager();                    // Resizes everything according to new dimensions
    void createOptions();                        // Creates option buttons at the bottom
    void helpLoop();                             // Loop when help option is pressed
    void infoLoop();                             // Loop when info option is pressed
    void mkdirLoop();                            // Loop when mkdir option is pressed
    void renameLoop();                           // Loop when rename option is pressed
    void deleteLoop();                           // Delete a file/folder loops for confirmation
    void attributeLoop();                        // Loop when change attribute option is selected
    WINDOW* printBindings();                     // Window on which to print the keybindings for help
    void copyItem();                             // Copies currently selected item
    void pasteItem();                            // Pastes currently selected item
    void errorLoop(char* msg);                   // Display error message
    char* trimFormWhitespaces(char* str);        // Trims the whitespaces created by forms

    char options[10][10];                        // Names of all the options at the bottom
    int optionsPadding;                          // Padding for each option

    char copyPathBuffer[maxPathLength];          // Buffer for the path of the selected item to paste later
    char copyFileNameBuffer[100];                // Buffer to store the name of the file we are copying

    enum optionsMode                             // Enum that describes which option is currently running
    {
        none,
        helpOption,
        copyOption,
        cutOption,
        moveOption,
        deleteOption,
        mkdirOption,
        renameOption
    };

    optionsMode currentOption;
    
    
public:
    FileManager(char* rootDir);
    ~FileManager();

    void mainEventLoop();                        // Main event loop that keeps the program running
};



#endif