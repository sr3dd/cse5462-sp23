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

int receive_data(int socket){

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

  printf ("received '%s'\n", bufferReceived);

  return rc;
}

int send_data(int socket, char *server_ip, int port_num){

  // structure for storing destination server address and port number
  struct sockaddr_in server_address = { 
    .sin_family = AF_INET,
    // convert from host byte ordering to network byte ordering
    .sin_port = htons(port_num),
    // convert string IP address to 32 bit IPv4 address
    .sin_addr.s_addr = inet_addr(server_ip)
  };

  // creating a 100 byte sized buffer to send
  char bufferOut [100];
  // clearing buffer
  memset (bufferOut, 0, 100);
  // loading buffer with transmit data
  sprintf (bufferOut, "hello world");

  int rc = sendto(socket, bufferOut, strlen(bufferOut), 0,
	      (struct sockaddr *) &server_address, sizeof(server_address));

  // checking if number of bytes sent is less than bytes in buffer
  if (rc < (int)strlen(bufferOut)){
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
