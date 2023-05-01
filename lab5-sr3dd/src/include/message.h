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
struct list *parse_payload(char const *);
// extracts key and value strings from a given string
struct pair *parse_kv(char *);
// validate message key-val pairs 
bool is_valid_message(struct list *);
// print message key-value pairs
void print_key_value_pairs(void *);
// take user inputted string and append protocol specific key-val pairs:
// location, fromPort, TTL
char *message_constructor(const char *, int, int, int);
// print the key-value list extracted from message payload
void print_payload(struct list *);
// construct a message string given a list of key-value pairs
char *construct_str_message(struct list *);
// match a key-value node in a list by key
bool is_key_match(void *, void *);

#endif
