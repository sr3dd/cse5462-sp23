#include "list.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct list *init_list(size_t size){
    struct list *l = malloc(sizeof(struct list));

    l->head = NULL;
    l->tail = NULL;
    l->length = 0;
    l->type_size = size;

    return l;
}

struct node *create_node(void *node_data){
    struct node *n = malloc(sizeof(struct node));
    
    n->data = node_data;
    n->next = NULL;

    return n;
}

void prepend(struct list *l, void *data){
    struct node *n = create_node(data);
    
    n->next = l->head;
    l->head = n;
    
    if(!l->tail){
        l->tail = n;
    }

    l->length++;
}

void append(struct list *l, void *data){
    struct node *n = create_node(data);
    
    if(!l->head){   // list is empty to begin with
        l->head = l->tail = n;
    }
    else{   // list is not empty
        l->tail->next = n;
        l->tail = n;
    }

    l->length++;
}

void print(struct list *l, void (*print)(void *)){
    struct node *tmp = l->head;

    if(!tmp){
        printf("List is empty!\n");
        return;
    }

    while(tmp){
        print(tmp->data);
        tmp = tmp->next;
    }
}

struct node *delete(struct list *l, void *data, bool (*is_equal)(void *, void *)){
    struct node *tmp = l->head;

    while(tmp){
        if(is_equal(tmp->data, data)){
            
            // only node in list
            if(l->tail == l->head){
                l->head = NULL;
                l->tail = NULL;
                l->length--;
                break;
            }

            // first node in list
            if(l->head == tmp){
                l->head = tmp->next;
                tmp->next = NULL;
                l->length--;
                break;
            }

            // last node in list
            if(l->tail == tmp){
                struct node *t = l->head;

                while(t->next != tmp){
                    t = t->next;
                }

                l->tail = t;
                t->next = NULL;
                l->length--;
                break;
            }

            //node in the middle of the list
            struct node *t = l->head;

            while(t->next != tmp){
                t = t->next;
            }

            t->next = tmp->next;
            tmp->next = NULL;
            l->length--;
            break;
        }

        tmp = tmp->next;
    }

    if(tmp){
        return tmp->data;
    }
    else{
        return NULL;
    }
}
