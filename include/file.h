#ifndef FILE_H
#define FILE_H

#include <sys/stat.h>
#include <sys/types.h>


class File
/**
 * This class provides a data type for files on the hard drive
 * It describes the files attributes, permissions etc.
 * It is the base class for the Directory class
 */
{
protected:
    int scrollOffset;                      // Keeps track of how much we've scrolled when we UP--DIR
    int item_ptr;                          // Keeps track of which item we were on when we go UP--DIR

public:
    File(char* name, char* type, char size[8], int sizeRaw, bool hidden, char permissions[10],
         char timeModified[15], char timeAccessed[15], int timeModifiedRaw, char* extension);
    File(const File& file);
    ~File();
    
    char* name;                             // Name of the file
    char* type;                             // Type of file
    char size[8];                           // Size of the file (in Bytes)
    int sizeRaw;                            // Real size in number format
    bool hidden;                            // Whether the file is hidden or not
    char permissions[10];                   // Read, write, execute. For Owner, Group, Others
    char timeModified[15];                  // Last time this file was modified. Format: dd-mm-yy hh:mm
    char timeAccessed[15];                  // Last time the file was accessed. Format: dd-mm-yy hh:mm
    int timeModifiedRaw;                    // Raw expression for time in seconds
    char* extension;                        // Extension of given file

    int getScrollOffset();                  // Returns scroll offset for current file      
    void setScrollOffset(int n);            // Sets scroll offset for current file        
    int getItem_ptr();                      // Returns item_ptr for current file
    void setItem_ptr(int n);                // Sets item_ptr for current file          
};



#endif