#define  _GNU_SOURCE
#include "common.h"
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

// Payload linked list implementation
struct payload_node *init_payload_list(void){
  struct payload_node *head = NULL;
	return head;
}

void add_payload_node(struct payload_node **payload_list, struct kv_node *kv_list){
  struct payload_node *new_node = malloc(sizeof(struct payload_node));

  // checking condition for payload extraction failure
  if(kv_list == NULL){
    return;
  }

	new_node->payload = kv_list;
	new_node->next = NULL;

	if(*payload_list == NULL){
		*payload_list = new_node;
		return;
	}

	struct payload_node *tmp = *payload_list;

	while(tmp->next != NULL){
		tmp = tmp->next; 
	}

	tmp->next = new_node;
	return;
}


// KeyValue pair linked list  implementation
struct kv_node *init_kv_list(){
	struct kv_node *head = NULL;
	return head;
}

void add_kv_node(struct kv_node **kv_list, char *key_str, char *value_str){
	struct kv_node *new_node = malloc(sizeof(struct kv_node));

	new_node->key = key_str;
	new_node->value = value_str;
	new_node->next = NULL;

	if(*kv_list == NULL){
		*kv_list = new_node;
		return;
	}

	struct kv_node *tmp = *kv_list;

	while(tmp->next != NULL){
		tmp = tmp->next; 
	}

	tmp->next = new_node;
	return;
}

void print_kv_list(struct kv_node *kv_list){
	struct kv_node *tmp = kv_list;

	while(tmp != NULL){
		printf("\t%-20s", tmp->key);
		printf("\t%-20s", tmp->value);
		printf("\n");
		tmp = tmp->next; 
	}

	return;
}


// Validation helper functions

void validate_server_args(int argc, char **argv, int *portNum)
{
  //set host type
  enum host_type host = SERVER;
  // validate number of args provided
  validate_arg_count(argc, host);
  // validate user port string
  validate_port_str(argv[1]);
  // convert port string to a number
  *portNum = strtol(argv[1], NULL, 10);
  //validate port value (0<=port_number<65536)
  validate_port_val(*portNum);
}

void validate_client_args(int argc, char **argv, int *portNum, char *serverIP)
{
  // set host type
  enum host_type host = CLIENT;
  // validate number of args provided
  validate_arg_count(argc, host);
  // validate user string containing IP address
  validate_ip_addr(argv[1]);
  // extract user IP address string 
  strcpy(serverIP, argv[1]);
  // validate user port string
  validate_port_str(argv[2]);
  // convert port string to a number
  *portNum = strtol(argv[2], NULL, 10);
  // validate port value (0<=port_number<65536)
  validate_port_val(*portNum);
}


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

struct kv_node *receive_data(int socket){

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

  // print received string
  printf ("Received '%s'\n", bufferReceived);
  // print client IP address
  char *ip_addr = inet_ntoa(from_address.sin_addr);
  printf("From IP: %s\n", ip_addr);
  // print size of received string in bytes
  printf("Size: %zd bytes\n", sizeof(char)*strlen(bufferReceived));
  
  // extract key-value pairs from payload into a linked list
  struct kv_node *list = parse_payload(bufferReceived);

  if(list == NULL){
    printf("Message payload parsing failed due to invalid payload\n");
    return list;
  }

  printf("*****************************************************************\n");
  // print key-value pairs
  print_kv_list(list);
  printf("*****************************************************************\n");
  return list;
}

struct kv_node *parse_payload(char *buffer){
	struct kv_node *head = init_kv_list();

  // check if received string contains the '^' character making it invalid
	if(strcspn(buffer, "^") < strlen(buffer)){
		printf("Invalid payload: string contains a '^' character\n");
		return head;
	}

	int count_messages = 0;

  // Replace spaces between quotes with ^ character
	for(unsigned int i=0, j=0; i<strlen(buffer); i++){
		if(buffer[i] == '\"'){
			++j;
			count_messages++;
			continue;
		}

		if(j%2 != 0){
			if(buffer[i] == ' '){
				buffer[i] = '^';
			}
		}
	}

  // Check if unbounded quote character exists
	if(count_messages  == 1){
		printf("Invalid payload: string contains a single unclosed quote\n");
		return head;
	}

  // Check if payload contains more than one msg key-value pairs - not allowed
	if(count_messages > 2){
		printf("Invalid payload: string contains more than one quoted value pairs\n");
		return head;
	}

	char sstr[100];
	char *token;
  // Tokenize string based on whitespace character
	token = strtok(buffer, " ");
	while(token != NULL){
		strcpy(sstr, token);
		char *key = NULL;
		char *val = NULL;
    // extract key-value pair from token
		parse_kv(&key, &val, sstr);
		
    // Invalid key in key-value pair
		if(key==NULL){
			token = strtok(NULL, " ");
			continue;
		}
		
    // Add key-value pair as a node in linked list
		add_kv_node(&head, key, val);
		token = strtok(NULL, " ");
	}

	return head;
}

void parse_kv(char **key, char **val, char* sstr){

  // Check if last character of token is : character
  // this means there is no key in token
  // invalid key-value pair
	if(strcspn(sstr, ":") == strlen(sstr)){
		printf("Valid format is <key>:<value> not %s\n", sstr);
		return;
	}

  // Check if first character of token is : character
  // this means there is no key in token
  // invalid key-value pair
	if(strcspn(sstr, ":") == 0){
		printf("Valid key does not exist: %s\n", sstr);
		return;
	}

  // get position of : character to break token into key and value pairs
	unsigned int kv_delim = strcspn(sstr, ":");
	
	*key = malloc(sizeof(char)*(kv_delim+1));	
	strncpy(*key, sstr, kv_delim);
	(*key)[kv_delim] = '\0';

  *val = malloc(sizeof(char)*(strlen(sstr)-kv_delim+1));
	strncpy(*val, &sstr[kv_delim+1], strlen(sstr)-kv_delim);
  (*val)[strlen(sstr)-kv_delim] = '\0';

  if(strcspn(*val, "\"") < strlen(*val) && (strcmp(*key, "msg")) != 0){
	  printf("Invalid! A quoted value can only belong to a 'msg' key : %s\n", sstr);
    *key = NULL;
	  return;
	}
  // if key is "msg", then replace ^ with whitespace characters again
	if((strcmp(*key, "msg")) == 0){
		for(unsigned int i=0; i<strlen(*val); i++){
			if((*val)[i] == '^'){
				(*val)[i] = ' ';
			}
		}
	}

	// printf("key: %s\t", key);
	// printf("value: %s\n", val);

	return;
}

void print_kv_pairs(char *bufferReceived){
	printf("*****************************************************************\n");

  	// remove trailing newline character if not handled by client
  	bufferReceived[strcspn(bufferReceived, "\n")] = '\0';

  	int i_delim[30] = {0};

  	// finding all whitespace indices which are not enclosed by parenthesis
  	for(unsigned int i=0, j=0, k = 0; i < strlen(bufferReceived); i++){
    	if(bufferReceived[i] == '\"'){
      	++j;
    	}
    	if(bufferReceived[i] == ' '){
      		if(j%2 == 0){
        		i_delim[k++] = i;
      		}
    	}
  	}

  	char sstr[50];
  	printf("\t%-20s\t%-20s\n", "Name", "Value");

  	// tokenize based on whitespace ' ' character
  	for(unsigned int i=0; i<30; i++){
    
    	if(i_delim[i+1] == 0){
      	unsigned int cpy_len = strlen(bufferReceived)-i_delim[i];
      	strncpy(sstr, &bufferReceived[i_delim[i]+1], cpy_len);
      	sstr[cpy_len] = '\0';
    	}
    	else if(i == 0){
      	strncpy(sstr, bufferReceived, i_delim[i]);
      	sstr[i_delim[i]] = '\0';
    	}
    	else{
      		if(i_delim[i] == i_delim[i-1]+1){
        	continue;
      		}
      	unsigned int cpy_len = i_delim[i]-i_delim[i-1];
      	strncpy(sstr, &bufferReceived[i_delim[i-1]+1], cpy_len);
      	sstr[cpy_len] = '\0';
    	}

    	//printf("%s\n", sstr);
    	char bstr[50];

    	unsigned int kv_delim = strcspn(sstr, ":");
    	strncpy(bstr, sstr, kv_delim);
    	bstr[kv_delim] = '\0';
    	printf("\t%-20s", bstr);

    	strncpy(bstr, &sstr[kv_delim+1], strlen(sstr)-kv_delim);
    	bstr[strlen(sstr)-kv_delim] = '\0';
    	printf("\t%-20s\n", bstr);

    	if(i_delim[i+1] == 0){
      	break;
    	}
  	}

  	printf("*****************************************************************\n");
}

char **parse_lines(char *file_name){
  // file descriptor
  FILE *fp;

  // Attempt to open file corresponding to given filename
  if((fp = fopen(file_name, "r")) == NULL){
    printf("Could not open file: %s\n", file_name);
    return NULL;
  }

  // declare pointers to pointer pointing to strings
  // NLINE_MAX dictates the number of input lines read from file
  char **lines = (char **)calloc(NLINE_MAX, sizeof(char *));
  char *line_str = NULL;
  size_t line_len = 0;
  ssize_t read_len;

  int i = 0;

  while((read_len = getline(&line_str, &line_len, fp)) != -1){
    // allocate a string of read line length
    char *line = malloc(sizeof(char)*line_len);
    // remove the '\n' at the end of line input if it exists
    line_str[strcspn(line_str, "\r\n")] = '\0';
    // copy into line so line_str can be reused for next read
    strcpy(line, line_str);
    lines[i] = line;
    i++;
    // Make sure we read only NLINE_MAX lines
    if(i == NLINE_MAX){
      printf("This client is designed to read only %d lines \
                        of input\n", NLINE_MAX);
      break;
    }
  }

  return lines;
}

int send_data(int socket, char *server_ip, int port_num, char *str_payload){
  int rc;
  // structure for storing destination server address and port number
  struct sockaddr_in server_address = { 
    .sin_family = AF_INET,
    // convert from host byte ordering to network byte ordering
    .sin_port = htons(port_num),
    // convert string IP address to 32 bit IPv4 address
    .sin_addr.s_addr = inet_addr(server_ip)
  };

  // creating a 100 byte sized buffer to send
  char bufferOut [1000];
  memset (bufferOut, 0, 1000);
  strcpy(bufferOut, str_payload);
  
  printf("Sending data: %s\n", str_payload);
  rc = sendto(socket, bufferOut, strlen(bufferOut), 0,
	      (struct sockaddr *) &server_address, sizeof(server_address));

  // checking if number of bytes sent is less than bytes in buffer
  if(rc < (int)strlen(bufferOut)){
    perror ("sendto");
    // exit program if whole data not sent out once, ideal: send the rest next
    exit(1);
  }

  return rc;
}

void validate_arg_count(int arg_count, enum host_type host){

    switch (host){
        case SERVER:{
            if (arg_count < 2){
                printf("usage is: server <portnumber>\n");
                exit (1);
            }
            break;
        }   
        case CLIENT:{
            if (arg_count < 3){
                printf ("usage is client <ipaddr> <portnumber>\n");
                exit(1);
            }
        }
        default:
            break;
        }
}

void validate_port_str(const char port_str[]){
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

void validate_ip_addr(const char ip_str[]){
    struct sockaddr_in inaddr;
 
    if (!inet_pton(AF_INET, ip_str, &inaddr)){
        printf ("error, bad ip address\n");
        exit (1);
    }
}
