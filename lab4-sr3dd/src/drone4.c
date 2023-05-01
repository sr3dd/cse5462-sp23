#include "list.h"
#include "config.h"
#include "location.h"
#include "message.h"
#include "udp_socket.h"
#include "validator.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#define STDIN 0

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("usage is: server <portnumber>\n");
        fflush(stdout);
        exit (1);
    }

    // command line port string validation
    validate_port_str(argv[1]);
    int port  = strtol(argv[1], NULL, 10);
    validate_port_val(port);

    // read the config line into a linked list 
    struct list *config_list = read_config_file(CONFIG_FILE_NAME);

    // If reading file name from default file path fails, prompt user
    while(!config_list){
        printf("Failed to read config file, enter config file to read: ");
        fflush(stdout);
        char file_name[20];
        scanf("%s", file_name);
        config_list = read_config_file(file_name);
    }

    // extract server config info from the config file, used later
    struct config *self_addr = (struct config *)delete(config_list, &port, match_by_port);

    // Check that port given as command line argument actually corresponds
    // to a row in the configuration line, else notify user and quit
    if(!self_addr){
        printf("Given port: %d is not present in config file\n", port);
        fflush(stdout);
        exit(1);
    }

    int rows = 0, cols = 0;

    // Prompt user to input number of rows and columns in the network grid
    do{
        printf("Enter rows: ");
        fflush(stdout);
        scanf("%d", &rows);
        printf("Enter columns: ");
        fflush(stdout);
        scanf("%d", &cols);
    }while((rows<=0)||(cols<=0)); // re-prompt until rows/cols are valid

    if(rows*cols < self_addr->location){
        printf("The location of server is not in grid\n");
        fflush(stdout);
        exit(1);
    }
    
    // server socket to listen to incoming messages
    int socket_fd = create_socket();
    bind_socket(socket_fd, port);

    // client socket to send messages to other servers
    int send_socket_fd = create_socket();

    char buffer[1000];

    // a file descriptor set which select() will mask
    fd_set socketFDS;

    // a linked list to store all valid message payloads received by server
    struct list *payload_list = init_list(sizeof(struct list));

    printf("Ready to accept user input\n");
    
    while(1){
        memset (buffer, 0, 1000);
        // do the select() stuff here
        // clear all fds from fd_set
        FD_ZERO(&socketFDS);
        // add socket FD to fd_set                           
        FD_SET(socket_fd, &socketFDS);
        // add stdin FD to fd_set
        FD_SET(STDIN, &socketFDS);
        // becaues socket_fd will always be greater than stdin fd
        select (socket_fd+1, &socketFDS, NULL, NULL, NULL); // NEW block until something arrives
        
        if (FD_ISSET(STDIN, &socketFDS)){
            printf("_________________________________________________________\n");
            printf("Received user input from STDIN: ");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strlen(buffer)-1] = '\0';
            printf ("'%s'\n", buffer);
            fflush(stdout);
            // in case user input string is empty
            if(strlen(buffer) == 0){
                printf("There's nothing to send!\n");
            }
            else{
                send_data(send_socket_fd, buffer, config_list, self_addr->location);
            }
        }
        
        fflush(stdout);
        
        if (FD_ISSET(socket_fd, &socketFDS)){
            printf("\n\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            printf("Received from network: ");
            struct list *kv_list = receive_data(socket_fd, port);

            if(kv_list){
                printf("My location:\t%d\n", self_addr->location);
                struct pair *msg_loc_kv = (struct pair *)find(kv_list, "location", is_key_match);
                int msg_loc = atoi(msg_loc_kv->value);
                
                if(get_coordinates(msg_loc, rows, cols).x == rows){
                    printf("NOT IN GRID\n");
                    fflush(stdout);
                    continue;
                }

                if(is_in_range(rows, cols, self_addr->location, msg_loc)){
                    printf("IN RANGE\n");
                    // store validate message payload
                    append(payload_list, kv_list);
                }
                else{
                    printf("NOT IN RANGE\n");
                }
            }
        }
        
        fflush(stdout);
    }

    return 0;
}
