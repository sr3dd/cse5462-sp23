#include "common.h"

int main(int argc, char *argv[])
{
  char serverIP[20];
  int portNumber = 0;

  // validate arguments and extract server IP address and port number
  validate_client_args(argc, argv, &portNumber, serverIP);
  
  // creating a datagram socket
  int sd = create_socket();

  // sending a string "hello world" to destination IP and port number
  send_data(sd, serverIP, portNumber);

  return 0;
}
