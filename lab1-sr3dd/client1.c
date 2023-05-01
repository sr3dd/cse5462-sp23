#include "common.h"
#include<stdio.h>

int main(int argc, char *argv[])
{
  char serverIP[20];
  int portNumber = 0;

  // validate arguments and extract server IP address and port number
  validate_client_args(argc, argv, &portNumber, serverIP);
  
  // creating a datagram socket
  int sd = create_socket();
  char file_input[50];
  char **lines;

  // Prompt user to enter a file name
  // An invalid file name or failure to read file will repeat the prompt
  do{
    printf("Enter file name: ");
    scanf("%s", file_input);
    lines = parse_lines(file_input);
  }while(lines == NULL);

  // Send each line of file to server
  for(int i=0; i<NLINE_MAX; i++){
    // exit-condition
    if(lines[i] == 0){
      break;
    }
    send_data(sd, serverIP, portNumber, lines[i]);
  }  

  return 0;
}
