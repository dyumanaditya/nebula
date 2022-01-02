#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "file.h"


class LinkedList
/**
 * This class is a linked list that contains the File type.
 * It is a container that will be used for displaying in the menus class
 */
{
private:
    struct node
    {
        File* data;                                 // Each linked list node contains a File
        node* next;                                 // Pointer to next node in linked list
    };  

    node* head;                                     // Head of the linked list
    node* tail;                                     // Tail of the linked list
    int length;                                     // Length of the linked list

    static char* lowerCase(char* s);                // Converts a string to lowercase

public: 
    LinkedList();   
    ~LinkedList();  

    File* operator[](int idx);                      // Overload indexing operator to act like array
    int getLength();                                // Returns length of linked list  
    void append(File* file);                        // Appends file to end of linked list
    void del(int idx);                              // Delete a particular index from linked list
    void clear();                                   // Clears linked list of all data
    void sortByName();                              // Sorts the linked list in alphabetic order.
    void sortBySize();                              // Sorts the linked list by the size of the files
    void sortByTimeModified();                      // Sorts the linked list by the time it was modified
    int find(char* name);                           // Finds the name of a file in the linked list and returns the index

};


#endif