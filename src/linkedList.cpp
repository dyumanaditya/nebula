#include "linkedList.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>



/**
 * Constructor for linked list.
 * Sets head and tail to NULL
 */
LinkedList::LinkedList() : head(nullptr), tail(nullptr), length(0)
{
}


/**
 * Destructor for linked list
 * Deletes all nodes created with "new"
 * Without memory leak
 */
LinkedList::~LinkedList()
{    
    clear();
}


/**
 * Appends File object to the end of linked list
 */
void LinkedList::append(File* file)
{
    length += 1;
    node* temp = new node;
    temp->data = file;
    temp->next = nullptr;

    if (head==nullptr)
    {
        head = temp;
        tail = temp;
    }
    else
    {
        tail->next = temp;
        tail = temp;
    }
}


/**
 * Overload indexing operator to act like array
 */
File* LinkedList::operator[](int idx)
{
    if (idx >= length)
    {
        printf("Linked list index %d is out of range\n", idx);
        exit(EXIT_FAILURE);
    }

    node* n = head;
    for (int i=0; i<idx; ++i)
    {
        n = n->next; 
    }

    return n->data;
}


/**
 * Returns length of linked list
 */
int LinkedList::getLength()
{
    return length;
}


/**
 * Deletes a particular index from linked list
 */
void LinkedList::del(int idx)
{
    if (idx >= length)
    {
        printf("Cannot delete linked list index %d. It is out of range\n", idx);
        exit(EXIT_FAILURE);
    }

    length -= 1;
    node* n = head;
    node* prev = nullptr;
    for (int i=0; i<idx; ++i)
    {
        prev = n;
        n = n->next;
    }
    

    // Delete first element?
    if (n==head)
    {
        if (tail==head)
        {
            // delete head->data;
            head = nullptr;
            tail = nullptr;
        }
        else
        {
            // delete head->data;
            head = head->next;
        }
    }

    else
    {
        if (n->next==nullptr)
        {
            // delete n->data;
            prev->next = nullptr;
            tail = prev;
        }
        else
        {
            prev->next = n->next;
            // delete n->data;
        }
    }
    
    
    delete n->data;
    delete n;
}


/**
 * Clears the Linked List of all data properly without memory leaks
 */
void LinkedList::clear()
{
    while (length>0)
    {
        del(0);
    }
}


void LinkedList::sortByName()
{
    File* arr[length];
    int len = length;
    for (int i=0; i<len; ++i)
    {
        arr[i] = operator[](i);
    }

    // Bubble sort
    for (int i=0; i<len; ++i)
    {
        for (int j=0; j<len-i-1; ++j)
        {
            char* name = lowerCase(arr[j]->name);
            char* nextName = lowerCase(arr[j+1]->name);
            if (strcmp(name, nextName) > 0)
            {
                File* temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
            delete[] name;
            delete[] nextName;
        }
    }

    // Clear the LL (but not the file data) and put back sorted data
    tail = nullptr;
    for (int i=0; i<len; ++i)
    {
        node* temp = head->next;
        delete head;
        head = temp;
    }
    length=0;
    head = nullptr;

    int upDir_idx;
    for (int i=0; i<len; ++i)
    {
        if (strcmp(arr[i]->name, "..")==0)
        {
            upDir_idx = i;
            break;
        }
    }

    append(arr[upDir_idx]);
    for (int i=0; i<len; ++i)
    {
        if (i==upDir_idx)
        {
            continue;
        }
        append(arr[i]);
    }
}


void LinkedList::sortBySize()
{
    File* arr[length];
    int len = length;
    for (int i=0; i<len; ++i)
    {
        arr[i] = operator[](i);
    }

    // Bubble sort
    for (int i=0; i<len; ++i)
    {
        for (int j=0; j<len-i-1; ++j)
        {
            if (arr[j]->sizeRaw > arr[j+1]->sizeRaw)
            {
                File* temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }

    // Clear the LL (but not the file data) and put back sorted data
    tail = nullptr;
    for (int i=0; i<len; ++i)
    {
        node* temp = head->next;
        delete head;
        head = temp;
    }
    length=0;
    head = nullptr;
    
    int upDir_idx;
    for (int i=0; i<len; ++i)
    {
        if (strcmp(arr[i]->name, "..")==0)
        {
            upDir_idx = i;
            break;
        }
    }

    append(arr[upDir_idx]);
    for (int i=0; i<len; ++i)
    {
        if (i==upDir_idx)
        {
            continue;
        }
        append(arr[i]);
    }
}


void LinkedList::sortByTimeModified()
{
    File* arr[length];
    int len = length;
    for (int i=0; i<length; ++i)
    {
        arr[i] = operator[](i);
    }

    // Bubble sort
    for (int i=0; i<len; ++i)
    {
        for (int j=0; j<len-i-1; ++j)
        {
            if (arr[j]->timeModifiedRaw > arr[j+1]->timeModifiedRaw)
            {
                File* temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }

    // Clear the LL (but not the file data) and put back sorted data
    tail = nullptr;
    for (int i=0; i<len; ++i)
    {
        node* temp = head->next;
        delete head;
        head = temp;
    }
    length=0;
    head = nullptr;
    
    int upDir_idx;
    for (int i=0; i<len; ++i)
    {
        if (strcmp(arr[i]->name, "..")==0)
        {
            upDir_idx = i;
            break;
        }
    }

    append(arr[upDir_idx]);
    for (int i=0; i<len; ++i)
    {
        if (i==upDir_idx)
        {
            continue;
        }
        append(arr[i]);
    }
}


char* LinkedList::lowerCase(char* s)
{
    char* name = new char[strlen(s)+1]; 
    for (int i=0; i<strlen(s); ++i)
    {
        name[i] = tolower(s[i]);
    }
    return name;
}


/**
 * Finds the name of a file in the linked list and returns the index
 */
int LinkedList::find(char* name)
{
    for (int i=0; i<length; ++i)
    {
        if (strcmp(operator[](i)->name, name)==0)
        {
            return i;
        }
    }
    return -1;
}