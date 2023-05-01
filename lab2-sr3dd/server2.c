#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Checks whether received message has only one "port" key and that 
// value matches the port of the server
_Bool is_valid_port(struct kv_node *payload, int port);

int main(int argc, char *argv[])
{  
  int portNumber = 0;
  // Initializing a payload linked list to store received payload from clients
  struct payload_node *payload_list = init_payload_list();

  // validate arguments and extract port number
  validate_server_args(argc, argv, &portNumber);

  // create a datagram socket
  int sd = create_socket();

  // bind socket to port number
  bind_socket(sd, portNumber);
  
  while(1){
    // Store payload containing linked list of key-value pairs in payload node
    struct kv_node *payload = receive_data(sd);

    if(payload == NULL){
      continue;
    }

    if(is_valid_port(payload, portNumber)){
    
      add_payload_node(&payload_list, payload);

      // print key-value pairs
      printf("*****************************************************************\n");
      print_kv_list(payload);
      printf("*****************************************************************\n");
    }
    else{
      printf("*****************************************************************\n");
      printf("Payload not addressed to this server.\n");
      printf("*****************************************************************\n");
    }
  }

  return 0;
}


_Bool is_valid_port(struct kv_node *payload, int port){

  struct kv_node *tmp = payload;
  unsigned int port_keys = 0;
  _Bool match = 0;

  while(tmp != NULL){
    if(strcmp(tmp->key, "port") == 0){
      port_keys++;
      if(atoi(tmp->value) == port){
        match = 1;
      }
    }
    tmp = tmp->next;
  }

  // Returns invalid if more than one "port" keys present in payload as per
  // Lab 2 assumptions
  if(port_keys > 1){
    return 0;
  }

  return match;
}
