#include "common.h"

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
      add_payload_node(&payload_list, receive_data(sd));
  }

  return 0;
}
