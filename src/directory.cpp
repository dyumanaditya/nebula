#include "directory.h"


Directory::Directory(char* name, char* type, char size[8], int sizeRaw, bool hidden, char permissions[10],
                     char timeModified[15], char timeAccessed[15], int timeModifiedRaw, char* extension)
    : File(name, type, size, sizeRaw, hidden, permissions, timeModified, timeAccessed, timeModifiedRaw, extension)
{
}


Directory::Directory(const File& directory) : File(directory)
{
}


Directory::~Directory()
{
}