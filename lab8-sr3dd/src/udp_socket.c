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
struct list *get_send_path_ports(const char *);
bool port_in_list(struct list *, int);

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
                    struct list *history, struct list *broadcast_history, 
                    int location, int client_port){
    
    enum message_type msg;

    if ((struct pair *)find(parse_payload(str_payload), "move", is_key_match)){
        msg = MOV;
    }
    else{
        msg = REG;
    }

    char *message;

    if(msg == MOV){
        message = move_message_constructor(str_payload, client_port, TTL, VERSION);

        if(!has_move_mandatory_keys(parse_payload(message))){
            printf("Cannot send this message, missing mandatory keys.\n");
            return;
        }
    }
    else{
        int to_port_kv = 
            atoi(((struct pair *)find(parse_payload(str_payload), 
                    "toPort", 
                        is_key_match))->value);
        struct history *port_record = 
                (struct history *)find(history, &to_port_kv, is_port_match);
        
        int seqNum = port_record ? port_record->seq_list->length + 1 : 1;

        // Creating a new node for this seqNum in the history list
        struct seq_node *new_seq = malloc(sizeof(struct seq_node));
        new_seq->seqNum = seqNum;
        new_seq->acked = false;

        // Updating the outgoing message history
        if(!port_record){
            struct history *new_port_record = malloc(sizeof(struct history));
            new_port_record->port = to_port_kv;
            new_port_record->seq_list = init_list(sizeof(struct seq_node));
            append(new_port_record->seq_list, new_seq);
            append(history, new_port_record);
        }
        else{
            append(port_record->seq_list, new_seq);
        }
    
        // appending location, fromPort and TTL key-val pairs to the message
        message = message_constructor(str_payload, 
                            seqNum, location, client_port, TTL, VERSION);

        if(!has_mandatory_keys(parse_payload(message))){
            printf("Cannot send this message, missing mandatory keys.\n");
            return;
        }
    }
    
    struct node *addr_node = recipient_list->head;

    append(broadcast_history, parse_payload(message));

    printf("Message: %s\n", message);
    printf("Broadcasting to %d recipients from config file\n", recipient_list->length);
    

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
                        struct list *kv_list, struct list *broadcast_list,
                        int location){
    
    struct pair *msg_loc_kv = (struct pair *)find(kv_list, "location", is_key_match);
    struct pair *msg_ttl_kv = (struct pair *)find(kv_list, "TTL", is_key_match);
    struct pair *msg_send_path_kv = (struct pair *)find(kv_list, "send-path", is_key_match);
    struct list *prev_port_list = get_send_path_ports(msg_send_path_kv->value);

    char *loc = malloc(sizeof(char)*4);
    char *ttl = malloc(sizeof(char)*4);
    sprintf(loc, "%d", location);
    msg_loc_kv->value = loc;
    int new_ttl = atoi(msg_ttl_kv->value)-1;
    
    if(new_ttl < 0){
        delete(broadcast_list, kv_list, is_equal_message);
        return;
    }
    
    sprintf(ttl, "%d", new_ttl);
    msg_ttl_kv->value = ttl;
    
    char *payload_str = construct_str_message(kv_list);
    
    printf("Forwarding to %d recipients from config file\n", recipient_list->length);
    struct node *addr_node = recipient_list->head;
    printf("Message: %s\n", payload_str);

    while(addr_node){
        struct config *data = (struct config *)addr_node->data;
        if(!port_in_list(prev_port_list, data->port)){
            send_str(socket, data->port, data->ip_addr, payload_str);
        }
        addr_node = addr_node->next;
    }
}

void acknowledge(int socket_fd, struct list *config_list, 
                    struct list *kv_list, int location){

    struct pair *msg_loc_kv = (struct pair *)find(kv_list, "location", is_key_match);
    struct pair *msg_ttl_kv = (struct pair *)find(kv_list, "TTL", is_key_match);
    struct pair *msg_send_path_kv = (struct pair *)find(kv_list, "send-path", is_key_match);
    struct pair *to_port_kv = (struct pair *)find(kv_list, "toPort", is_key_match);
    struct pair *from_port_kv = (struct pair *)find(kv_list, "fromPort", is_key_match);

    struct pair *ack_type_node = malloc(sizeof(struct pair));
    ack_type_node->key = "type";
    ack_type_node->value = "ACK";

    append(kv_list, ack_type_node);
    
    char loc[4];
    sprintf(loc, "%d", location);
    msg_loc_kv->value = loc;
    
    char ttl[4];
    sprintf(ttl, "%d", TTL);
    msg_ttl_kv->value = ttl;

    char to_port[10];
    char from_port[10];
    strcpy(to_port, from_port_kv->value);
    strcpy(from_port, to_port_kv->value);
    
    msg_send_path_kv->value = from_port;
    to_port_kv->value = to_port;
    from_port_kv->value = from_port;

    char *payload_str = construct_str_message(kv_list);

    printf("ACKing a message...\n");
    printf("Forwarding to %d recipients from config file\n", config_list->length);
    struct node *addr_node = config_list->head;
    printf("Message: %s\n", payload_str);

    while(addr_node){
        struct config *data = (struct config *)addr_node->data;
        send_str(socket_fd, data->port, data->ip_addr, payload_str);
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
    //printf("Sending ...\n");
    //printf("Address IP: %s\tAddress Port: %d\n", server_ip, port_num);
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

struct list *get_send_path_ports(const char *send_path_str){
    struct list *port_list = init_list(sizeof(int));
    
    char *str_cpy = malloc(strlen(send_path_str));
    strcpy(str_cpy, send_path_str);

    int *new_port;

    if(strcspn(str_cpy, ",") >= strlen(str_cpy)){
        new_port = malloc(sizeof(int));
        *new_port = atoi(str_cpy);
		append(port_list, new_port);
        return port_list;
	}

    char *token;
    // Tokenize string based on comma character
	token = strtok(str_cpy, ",");
	while(token != NULL){
		// strcpy(sstr, token);
        // get port
		new_port = malloc(sizeof(int));
        *new_port = atoi(token);
        append(port_list, new_port);
		token = strtok(NULL, " ");
	}

    return port_list;
}

bool port_in_list(struct list *prev_port_list, int port){
    bool port_present = false;

    struct node *port_node = prev_port_list->head;

    while(port_node){
        int node_port = *((int *)port_node->data);
        if(port == node_port){
            return true;
        }
        port_node = port_node->next;
    }

    return port_present;
}
