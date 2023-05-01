#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdbool.h>

struct node{
    void *data;
    struct node *next;
};

struct list{
    struct node *head; // points to first element in LL
    struct node *tail; // points to last element of LL
    size_t type_size; // the size of data in node
    unsigned int length; // tracks the lenght of the LL
};

// Init a linked list of type having size 'size' and return pointer to list
struct list *init_list(size_t size);
// Create a node from given node_data
struct node *create_node(void *node_data);
// Add a node to head of the list
void prepend(struct list *l, void *data);
// Add a node to tail of the list
void append(struct list *l, void *data);
// print list, takes function pointer which can print given data in node
void print(struct list *l, void (*print)(void *));
// remove and return a node from list if it matches some data
struct node *delete(struct list *l, void *data, bool (*is_equal)(void *, void *));

#endif
