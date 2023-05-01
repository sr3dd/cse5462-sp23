#ifndef COMMON_H
#define COMMON_H

// enum type to specify host type
enum host_type {SERVER, CLIENT};

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

// create a socket
//returns socket FD
int create_socket(void);
// bind a given socket FD with given port number
// returns int which specifies success/failure
int bind_socket(int, int);
// send data from given socket FD with IP address and port number provided
// returns int - number of bytes actually sent of output buffer
int send_data(int, char *, int);
// receive data from given socket FD
//return bytes read into input buffer
int receive_data(int);

#endif
