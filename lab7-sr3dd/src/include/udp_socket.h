#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include "list.h"

// argument validation functions

// validate if user string can be port
void validate_port_str(const char *);
// validate if port number value is between 0 and 65536
void validate_port_val(int);

// socket helper functions

// create a socket
//returns socket FD
int create_socket(void);
// bind a given socket FD with given port number
// returns int which specifies success/failure
int bind_socket(int, int);
// send data from given socket FD with IP address and port number provided
// returns int - number of bytes actually sent of output buffer
void send_data(int, char *, struct list *, struct list *, int, int);
// receive data from given socket FD
//return bytes read into input buffer
struct list *receive_data(int);
// resend a received payload to all clients in config
void retransmit_data(int, struct list *, struct list *, int, int);
// acknowledge received payload
void acknowledge(int, struct list *, struct list *, int);

#endif
