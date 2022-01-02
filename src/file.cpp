#include "file.h"
#include <string.h>


File::File(char* name, char* type, char size[8], int sizeRaw, bool hidden, char permissions[10],
           char timeModified[15], char timeAccessed[15], int timeModifiedRaw, char* extension)
    : name(new char[strlen(name) + 1]), type(new char[strlen(type) + 1]), sizeRaw(sizeRaw), hidden(hidden),
      timeModifiedRaw(timeModifiedRaw), extension(new char[strlen(extension) + 1]), scrollOffset(0), item_ptr(0)
{
    strcpy(File::name, name);
    strcpy(File::type, type);
    strcpy(File::extension, extension);
    strcpy(File::size, size);
    strcpy(File::permissions, permissions);
    strcpy(File::timeModified, timeModified);
    strcpy(File::timeAccessed, timeAccessed);
}


File::File(const File& file) : name(new char[strlen(file.name)+1]), type(new char[strlen(file.type)+1]), sizeRaw(file.sizeRaw),
                               hidden(file.hidden), timeModifiedRaw(file.timeModifiedRaw), extension(new char[strlen(file.extension)+1]),
                               scrollOffset(file.scrollOffset), item_ptr(file.item_ptr)
{
    strcpy(name, file.name);
    strcpy(type, file.type);
    strcpy(extension, file.extension);
    strcpy(size, file.size);
    strcpy(permissions, file.permissions);
    strcpy(timeModified, file.timeModified);
    strcpy(timeAccessed, file.timeAccessed);
}


File::~File()
{
    delete[] type;
    delete[] name;
    delete[] extension;
}


/**
 * Returns scroll offset for current file
 */
int File::getScrollOffset()
{
    return scrollOffset;
}


/**
 * Sets scroll offset for current file
 */
void File::setScrollOffset(int n)
{
    if (strcmp(type, "Directory")==0)
    {
        scrollOffset = n;
    }
}


/**
 * Returns scroll offset for current file
 */
int File::getItem_ptr()
{
    return item_ptr;
}


/**
 * Sets scroll offset for current file
 */
void File::setItem_ptr(int n)
{
    if (strcmp(type, "Directory")==0)
    {
        item_ptr = n;
    }
}