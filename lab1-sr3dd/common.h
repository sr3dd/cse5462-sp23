#ifndef COMMON_H
#define COMMON_H

// number of lines to be read from file containing message payload
#define NLINE_MAX 10

// enum type to specify host type
enum host_type {SERVER, CLIENT};

// a linked list to store processed message payloads from clients
struct payload_node{
    struct kv_node *payload;
    struct payload_node *next;
};
// create a payload list with a head node
struct payload_node *init_payload_list(void);
// add a new payload containing list of key-value pairs
void add_payload_node(struct payload_node **payload_list, struct kv_node *kv_list);


// a linked list to store key-value pairs extracted from message payload
struct kv_node{
    char *key;
    char *value;
    struct kv_node *next;
};
// create a kv list with a head node
struct kv_node *init_kv_list(void);
// add a new key-value node containing key-value extracted from message string
void add_kv_node(struct kv_node **kv_list, char *key, char *value);
// print the key-value list constructed from message payload
void print_kv_list(struct kv_node *kv_list);

// argument validation functions

// validate and extract args for server
void validate_server_args(int, char **, int *);
// validate and extract args for client
void validate_client_args(int, char **, int *, char *);
// validate based on number of arguments provided by user
void validate_arg_count(int, enum host_type);
// validate if user string can be port
void validate_port_str(const char []);
// validate if port number value is between 0 and 65536
void validate_port_val(int);
// validate if user string is valid IP address
void validate_ip_addr(const char []);

// socket helper functions

// create a socket
//returns socket FD
int create_socket(void);
// bind a given socket FD with given port number
// returns int which specifies success/failure
int bind_socket(int, int);
// send data from given socket FD with IP address and port number provided
// returns int - number of bytes actually sent of output buffer
int send_data(int, char *, int, char *);
// receive data from given socket FD
//return bytes read into input buffer
struct kv_node *receive_data(int);

// string related parsing and validation functions

// parse string from given file name
char **parse_lines(char *);
// function that prints key-value pairs but does not store them
void print_kv_pairs(char *bufferReceived);
// function to parse message string into key-value pairs stored in linked list
struct kv_node *parse_payload(char *buffer);
// extracts key and value strings from a given string
void parse_kv(char **key, char **val, char *sstr);

#endif
