#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "file.h"


class Directory : public File
/**
 * This class inherits the File class and has all the properties of a file
 */
{
public:
    Directory(char* name, char* type, char size[8], int sizeRaw, bool hidden, char permissions[10],
              char timeModified[15], char timeAccessed[15], int timeModifiedRaw, char* extension);
    Directory(const File& directory);
    ~Directory();
};



#endif