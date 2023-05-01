#include "common.h"

int main(int argc, char *argv[])
{  
  int portNumber = 0;

  // validate arguments and extract port number
  validate_server_args(argc, argv, &portNumber);

  // create a datagram socket
  int sd = create_socket();

  // bind socket to port number
  bind_socket(sd, portNumber);
  
  // receive and print data
  receive_data(sd);
  
  return 0;
}
