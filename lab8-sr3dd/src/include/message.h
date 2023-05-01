#ifndef MESSAGE_H
#define MESSAGE_H

#include "list.h"
#include <stdbool.h>

#define MESSAGE_SIMILARITY_KEYS (char *[]){"fromPort", "toPort", "seqNumber"}

// enum to define different kinds of network messages
enum message_type {
    REG,
    ACK,
    MOV
};

// struct to hold a key-value pair from message parsing
struct pair{
    char* key;
    char* value;
};

// struct to hold a port address and corresponding messages received from it
struct history{
    int port;
    struct list *seq_list;
};

struct seq_node{
    int seqNum;
    bool acked;
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
// seqNum, location, fromPort, TTL
char *message_constructor(const char *, int, int, int, int, int);
// take userr inputted move command string and append key-val pairs:
// fromPort, TTL, version
char *move_message_constructor(const char *, int, int, int);
// print the key-value list extracted from message payload
void print_payload(struct list *);
// construct a message string given a list of key-value pairs
char *construct_str_message(struct list *);

// determine if received message is of type ACK
bool is_ack(struct list *);
// determine if received ACK is a duplicate
bool is_duplicate_ack(struct list *, struct list *);
// determine if received message is a duplicate
bool is_duplicate(struct list *, struct list *);

// match node by toPort
bool is_port_match(void *, void *);
// match seq_node by seqNum
bool is_seq_match(void *, void *);
// match a key-value node in a list by key
bool is_key_match(void *, void *);

bool is_equal_message(void *node_data, void *data);

// update history list with received message/ACK
void update_history(struct list *, struct list *, enum message_type);

#endif
