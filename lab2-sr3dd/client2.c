#include "common.h"
#include<stdio.h>
#include<stdlib.h>

int main(void)
{
  
  // read config file given as command-line arg
  char **config_lines = parse_lines(CONFIG_FILE_NAME);

  if(config_lines == NULL){
    printf("Could not read provided config file: %s\n", CONFIG_FILE_NAME);
    exit(1);
  }

  // read file which contains the messages to be sent
  char file_input[50];
  char **message_lines;
  
  do{
    printf("Enter file name: ");
    scanf("%s", file_input);
    message_lines = parse_lines(file_input);
  }while(message_lines == NULL);

  // creating a datagram socket
  int sd = create_socket();

  for(int i=0; i<NLINE_MAX; i++){
    if(message_lines[i]==0){
      break;
    }
    for(int j=0; j<NLINE_MAX; j++){
      if(config_lines[j]==0){
        break;
      }
      char *ip;
      int port;
      parse_address_line(&ip, &port, config_lines[j]);
      validate_port_val(port);
      validate_ip_addr(ip);

      send_data(sd, ip, port, message_lines[i]);
    }
  }

  return 0;
}
