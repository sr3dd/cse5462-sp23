#ifndef MESSAGE_H
#define MESSAGE_H

#include "list.h"
#include <stdbool.h>

// struct to hold a key-value pair from message parsing
struct pair{
    char* key;
    char* value;
};

// extract key-value pairs from message into a linked list
struct list *parse_payload(char *);
// extracts key and value strings from a given string
struct pair *parse_kv(char *);
// validate message key-val pairs 
bool is_valid_message(struct list *, int);
// print message key-value pairs
void print_key_value_pairs(void *);
// append ' location:<location>' to outgoing message string
char *append_location(const char *, int);
// print the key-value list extracted from message payload
void print_payload(struct list *);

#endif
