#include "directoryReader.h"
#include "directory.h"
#include "file.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>



/**
 * Constructor for DirectoryReader.
 * 
 * @param rootDir Path to the starting directory -- root directory
 */
DirectoryReader::DirectoryReader(char* rootDir)
{
    currentPath[0] = '\0';
    read(rootDir);
}


DirectoryReader::~DirectoryReader()
{
}


/**
 * Reads the contents of the directory and hands them to n-ary tree
 * 
 * @param dirName Name of the directory we want to read
 */
void DirectoryReader::read(char* dirName)
{
    changeDir(dirName);

    // Clear the linked lists
    files.clear();
    filesAndHiddenFiles.clear();

    DIR* dir;
    struct dirent* entry;
    struct stat statbuff;

    if ((dir = opendir(".")) == nullptr)
    {
        printf("Can't open directory");
        exit(EXIT_FAILURE);
    }

    // Go through all the files and folders in this dir
    while ((entry = readdir(dir)) != NULL)
    {
        stat(entry->d_name, &statbuff);
        
        // Get all file attributes
        int sizeRaw = statbuff.st_size;
        int timeModifiedRaw = statbuff.st_mtim.tv_sec;
        char* size = getSize(statbuff.st_size);
        char* type = getType(statbuff.st_mode);
        bool hidden = getHidden(entry->d_name);
        char* extension = getExtension(entry->d_name);
        char* permissions = getPermissions(statbuff.st_mode);
        char* timeModified = getFormattedTime(statbuff.st_mtim);
        char* timeAccessed = getFormattedTime(statbuff.st_atim);

        // Check whether this is a file or folder
        File* file;
        File* fileAndHiddenFile;
        if (statbuff.st_mode & S_IFDIR)
        {
            file = new Directory(entry->d_name, type, size, sizeRaw, hidden, permissions, timeModified, timeAccessed, timeModifiedRaw, extension);
            fileAndHiddenFile = new Directory(entry->d_name, type, size, sizeRaw, hidden, permissions, timeModified, timeAccessed, timeModifiedRaw, extension);

            if (strcmp(entry->d_name, ".") == 0)
            {
                continue;
            }
            else if (strcmp(entry->d_name, "..") == 0)
            {
                files.append(file);
                filesAndHiddenFiles.append(fileAndHiddenFile);
            }
            else if (!hidden)
            {
                files.append(file);
                filesAndHiddenFiles.append(fileAndHiddenFile);
            }
            else
            {
                filesAndHiddenFiles.append(fileAndHiddenFile);
            }
        }

        else
        {
            file = new File(entry->d_name, type, size, sizeRaw, hidden, permissions, timeModified, timeAccessed, timeModifiedRaw, extension);
            fileAndHiddenFile = new File(entry->d_name, type, size, sizeRaw, hidden, permissions, timeModified, timeAccessed, timeModifiedRaw, extension);
            if (!hidden)
            {
                files.append(file);
                filesAndHiddenFiles.append(fileAndHiddenFile);
            }
            else
            {
                filesAndHiddenFiles.append(fileAndHiddenFile);
            }
        }

        delete[] size;
        delete[] permissions;
        delete[] timeModified;
        delete[] timeAccessed;
    }
}


void DirectoryReader::changeDir(char* dirName)
{
    // First directory being read
    if (strlen(currentPath)==0)
    {
        strcpy(currentPath, dirName);
    }

    // Staying in same directory "."
    else if (strcmp(dirName, ".")==0)
    {
        return;
    }

    // UP -- DIR
    else if (strcmp(dirName, "..")==0)
    {
        // Go up one directory (unless it is root /)
        if (strcmp(currentPath, "/")!=0)
        {
            char* slash = strrchr(currentPath, '/');
            if (slash-currentPath==0)
            {
                *(slash+1) = '\0';
            }
            else
            {
                *slash = '\0';
            }
        }
        // Cannot go up further
        else
        {
            return;
        }
    }

    // We are in root
    else if (strcmp(currentPath, "/")==0)
    {
        strcat(currentPath, dirName);
    }

    // We are not in root
    else if (strcmp(currentPath, "/")!=0)
    {
        strcat(currentPath, "/");
        strcat(currentPath, dirName);
    }

    int result = chdir(currentPath);
    if (result!=0)
    {
        perror("Invalid directory, cannot chdir");
    }
}


char* DirectoryReader::getSize(int size)
{
    char* temp = new char[10];
    char* s = new char[8];
    sprintf(temp, "%d", size);

    int len = strlen(temp);
    if (len > 6)
    {
        int cnt = 0;
        while (len > 6)
        {
            cnt += 1;
            strncpy(s, temp, len-3);
            s[len-3] = '\0';
            len -= 3;
        }

        if (cnt == 1)
        {
            s[len] = 'K';
            s[len+1] = '\0';
        }
        else if (cnt == 2)
        {
            s[len] = 'M';
            s[len+1] = '\0';
        }
        else if (cnt == 3)
        {
            s[len] = 'G';
            s[len+1] = '\0';
        }
    }
    else
    {
        strncpy(s, temp, len);
        s[len] = '\0';
    }

    delete[] temp;
    return s;
}


char* DirectoryReader::getType(mode_t mode)
{
    if (mode & S_IFDIR)
    {
        return (char*)"Directory";
    }
    else if (mode & S_IFREG)
    {
        return (char*)"Regular File";
    }
    else if (mode & S_IFSOCK)
    {
        return (char*)"Socket";
    }
    else if (mode & S_IFLNK)
    {
        return (char*)"Symbolic Link";
    }
    else if (mode & S_IFBLK)
    {
        return (char*)"Block Device";
    }
    else if (mode & S_IFCHR)
    {
        return (char*)"Character Device";
    }
    else if (mode & S_IFIFO)
    {
        return (char*)"FIFO";
    }
    else
    {
        return (char*)"Unknown type";
    }
}


bool DirectoryReader::getHidden(char* name)
{
    if (name[0] == '.')
    {
        return true;
    }
    else
    {
        return false;
    }
}


char* DirectoryReader::getPermissions(mode_t mode)
{
    char* p = new char[10];

    // Owner permissions
    p[0] = (mode & S_IRUSR) ? 'r' : '-';
    p[1] = (mode & S_IWUSR) ? 'w' : '-';
    p[2] = (mode & S_IXUSR) ? 'x' : '-';

    // Group permissions
    p[3] = (mode & S_IRGRP) ? 'r' : '-';
    p[4] = (mode & S_IWGRP) ? 'w' : '-';
    p[5] = (mode & S_IXGRP) ? 'x' : '-';

    // Other permissions
    p[6] = (mode & S_IROTH) ? 'r' : '-';
    p[7] = (mode & S_IWOTH) ? 'w' : '-';
    p[8] = (mode & S_IXOTH) ? 'x' : '-';

    p[9] = '\0';

    return p;
}


char* DirectoryReader::getFormattedTime(timespec time)
{
    char* buffer = new char[15];
    tm* t = localtime(&time.tv_sec);
    strftime(buffer, 15, "%d-%m-%y %H:%M", t);
    return buffer;
}


char* DirectoryReader::getExtension(char* name)
{
    char* dot = strrchr(name, '.');
    if (!dot || strcmp(dot, name)==0 || strcmp(".", name)==0 || strcmp("..", name)==0)
    {
        return (char*)"";
    }
    else
    {
        return dot+1;
    }
}