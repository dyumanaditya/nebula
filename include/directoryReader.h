#ifndef DIRECTORY_READER_H
#define DIRECTORY_READER_H

#define maxPathLength 300

#include "linkedList.h"
#include "file.h"
#include <sys/stat.h>


class DirectoryReader
/**
 * This is the class responsible for reading the filesystem
 * and placing it into linked lists that will be accessed and displayed
 * by the menus class.
 */
{
private:
    void changeDir(char* dirName);                 // Changes the current directory and updates currentPath
    char* getSize(int size);                       // Given a file, gives you formatted size as a string   
    char* getType(mode_t mode);                    // Given a file, gives the type of file
    bool getHidden(char* name);                    // Given a file, gives you whether it is hidden or not
    char* getPermissions(mode_t mode);             // Given a file, gives the permissions in string form
    char* getFormattedTime(timespec time);         // Given a file, gives the time modified or the time accessed
    char* getExtension(char* name);                // Returns the extension of a file
    
public:
    char currentPath[maxPathLength];               // The path we are currently looking at in the menu. Not more than 300 chars
    DirectoryReader(char* rootDir);                // Constructor. Takes in the root directory
    ~DirectoryReader();
    
    LinkedList files;                              // Linked list that stores all the files and directories
    LinkedList filesAndHiddenFiles;                // Linked list that stores all the hidden files hidden directories + files

    void read(char* dirName);                      // Reads the directory that the user has clicked on
};



#endif