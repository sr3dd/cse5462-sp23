#define  _GNU_SOURCE // to suppress GCC compilation warnings

#include "udp_socket.h"
#include "message.h"
#include "list.h"
#include "config.h"
#include "validator.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

//private functions

int send_str(int, int, char *, const char *);

// socket helper functions

int create_socket(void){
    // Creating a datagram socket of type AF_INET/PF_INET
    // No need to bind socket to port number on client
    int sd = socket(AF_INET, SOCK_DGRAM, 0);
  
    // checking return value for error while creating socket
    if (sd == -1){
        perror ("socket");
        exit(1);
    }

    return sd;
}

int bind_socket(int socket, int port_num){
    // create address structure to store server port and IP
    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        // convert from host byte ordering to network byte order
        .sin_port = htons(port_num),
        // bind to any server adapter/address
        .sin_addr.s_addr = INADDR_ANY
    };
 
    int rc = bind (socket, (struct sockaddr *)&server_address,
	                        sizeof(struct sockaddr));
  
    // bind returns -1 on failure
    // failure could be caused by port already being used by other process
    if (rc < 0){
        perror("bind");
        exit (1);
    }

    return rc;
}

void send_data(int socket, char *str_payload, struct list *recipient_list, 
                    int location, int client_port, int ttl){
    
    printf("Sending to %d recipients from config file\n", recipient_list->length);

    // appending location, fromPort and TTL key-val pairs to the message
    char *message = message_constructor(str_payload, location, client_port, ttl);
    struct node *addr_node = recipient_list->head;

    if(!has_mandatory_keys(parse_payload(message))){
        printf("Cannot send this message, missing mandatory keys.\n");
        return;
    }

    printf("Sending message: %s\n", message);

    while(addr_node){
        struct config *data = (struct config *)addr_node->data;
        send_str(socket, data->port, data->ip_addr, message);
        addr_node = addr_node->next;
    }
}

struct list *receive_data(int socket){

    // buffer to store incoming data
    char bufferReceived[1000];
    memset (bufferReceived, 0, 1000);

    // stores length of the origin address
    socklen_t fromLength = sizeof(struct sockaddr_in);
    // struct to store origin address
    struct sockaddr_in from_address;
    int flags = 0;

    int rc = recvfrom(socket, bufferReceived, 1000, flags,
            (struct sockaddr *)&from_address, &fromLength);

    // check if there was an error: -1
    // check if termination from origin: 0
    if (rc <=0){
        perror ("recvfrom");
        printf ("leaving, due to socket error on recvfrom\n");
        exit (1);
    }

    printf ("'%s'\n", bufferReceived);
    // print client IP address
    char *ip_addr = inet_ntoa(from_address.sin_addr);
    printf("From IP: %s\n", ip_addr);
    // print size of received string in bytes
    printf("Size: %zd bytes\n", sizeof(char)*strlen(bufferReceived));

    //extract key-value pairs from payload into a linked list
    struct list *kv_list = parse_payload(bufferReceived);

    if(kv_list == NULL){
        printf("Message payload parsing failed\n");
    }

    return kv_list;
}

void retransmit_data(int socket, struct list *recipient_list, 
                        struct list *kv_list, int location){
    
    struct pair *msg_loc_kv = (struct pair *)find(kv_list, "location", is_key_match);
    struct pair *msg_ttl_kv = (struct pair *)find(kv_list, "TTL", is_key_match);
    
    char loc[4];
    sprintf(loc, "%d", location);
    msg_loc_kv->value = loc;
    char ttl[4];
    sprintf(ttl, "%d", atoi(msg_ttl_kv->value)-1);
    msg_ttl_kv->value = ttl;
    
    char *payload_str = construct_str_message(kv_list);
    
    printf("Retransmitting to %d recipients from config file\n", recipient_list->length);
    struct node *addr_node = recipient_list->head;
    printf("Sending message: %s\n", payload_str);

    while(addr_node){
        struct config *data = (struct config *)addr_node->data;
        send_str(socket, data->port, data->ip_addr, payload_str);
        addr_node = addr_node->next;
    }
}

// Validation helper functions

void validate_port_str(const char *port_str){
    for (unsigned int i=0;i<strlen(port_str); i++){
    if (!isdigit(port_str[i])){
	      printf ("The Portnumber isn't a number!\n");
	      exit(1);
      }
  }
}

void validate_port_val(int port_num){
    if ((port_num > 65535) || (port_num < 0)){
    printf ("you entered an invalid port number\n");
    exit (1);
  }
}

// private functions definitions

int send_str(int socket, int port_num, char *server_ip, const char *str_payload){

    int rc;
    // structure for storing destination server address and port number
    struct sockaddr_in server_address = { 
        .sin_family = AF_INET,
        // convert from host byte ordering to network byte ordering
        .sin_port = htons(port_num),
        // convert string IP address to 32 bit IPv4 address
        .sin_addr.s_addr = inet_addr(server_ip)
    };
  
    // print sending information
    printf("Sending ...\n");
    printf("Address IP: %s\tAddress Port: %d\n", server_ip, port_num);
    rc = sendto(socket, str_payload, strlen(str_payload), 0,
	            (struct sockaddr *) &server_address, sizeof(server_address));

    // checking if number of bytes sent is less than bytes in buffer
    if(rc < (int)strlen(str_payload)){
        perror ("sendto");
        // exit program if whole data not sent out once, ideal: send the rest next
        exit(1);
    }

    return rc;
}
