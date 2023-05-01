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
        exit(1);
    }

    int rows = 0, cols = 0;

    // Prompt user to input number of rows and columns in the network grid
    do{
        printf("Enter rows: ");
        scanf("%d", &rows);
        printf("Enter columns: ");
        scanf("%d", &cols);
    }while((rows<=0)||(cols<=0)); // re-prompt until rows/cols are valid

    if(rows*cols < self_addr->location){
        printf("The location of server is not in grid\n");
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
    
    // a linked list to track outgoing messages and associated port, seqNum and 
    // ACK status
    struct list *in_history_list = init_list(sizeof(struct history));
    
    // a linked list to track outgoing messages and associated port, seqNum and 
    // ACK status
    struct list *out_history_list = init_list(sizeof(struct history));

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
            // in case user input string is empty
            if(strlen(buffer) == 0){
                printf("There's nothing to send!\n");
            }
            else{
                send_data(send_socket_fd, buffer, config_list, 
                            out_history_list, self_addr->location, port, TTL);
            }
        }
                
        if (FD_ISSET(socket_fd, &socketFDS)){
            printf("\n\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            printf("Received from network: ");
            struct list *kv_list = receive_data(socket_fd);

            if(kv_list){

                // Message Validation fails
                if(!is_valid_message(kv_list)){
                    printf("Message invalidated due to key-value pair validation failure");
                    continue;
                }

                struct pair *msg_loc_kv = (struct pair *)find(kv_list, "location", is_key_match);
                struct pair *msg_ttl_kv = (struct pair *)find(kv_list, "TTL", is_key_match);
                int msg_loc = atoi(msg_loc_kv->value);
                int msg_ttl = atoi(msg_ttl_kv->value);
                
                bool is_valid_port = valid_port(kv_list, port);
                bool in_range = is_in_range(rows, cols, self_addr->location, msg_loc);
                
                // Message location not in grid
                if(get_coordinates(msg_loc, rows, cols).x == rows){
                    printf("NOT IN GRID\n");
                    continue;
                }

                // Good Message (in-range, right port, valid message) Processing
                if(is_valid_port && in_range && (msg_ttl >= 0)){
                    
                    // Received message is an ACK
                    if(is_ack(kv_list)){

                        // ACK is for a port or seqNum that has already been received
                        if(is_duplicate_ack(kv_list, out_history_list)){
                            printf("Duplicate ACK received! Discarding...\n");
                        }
                        
                        // new ACK received
                        else{
                            printf("Received an ACK:\n");
                            printf("My location:\t%d\n", self_addr->location);
                            print_payload(kv_list);
                            update_history(kv_list, out_history_list, ACK);
                        }

                        continue;
                    }

                    // Message seqNumber already received from fromPort
                    if(!is_duplicate(kv_list, in_history_list)){
                        printf("Received a mesage:\n");
                        printf("My location:\t%d\n", self_addr->location);
                        print_payload(kv_list);
                        // store validate message payload
                        append(payload_list, kv_list);
                        // update the incoming history list
                        update_history(kv_list, in_history_list, REG);
                        // acknowledge message receipt
                        acknowledge(send_socket_fd, config_list, 
                                        kv_list, self_addr->location);
                    }
                    
                    // Message seqNumber encountered first time from fromPort
                    else{
                        printf("Duplicate Received! fromPort: %s\tseqNumber: %s\n",
                            ((struct pair *)find(kv_list, "fromPort", is_key_match))->value,
                            ((struct pair *)find(kv_list, "seqNumber", is_key_match))->value);
                        // acknowledge the duplicate message as well
                        acknowledge(send_socket_fd, config_list, 
                                        kv_list, self_addr->location);
                    }

                    continue;
                }
                // Right Port BUT Not in Range
                else if(is_valid_port && !in_range){
                    // Message is out of range
                    ;
                }
                // Wrong Port AND in range AND TTL>0
                else if(!is_valid_port && in_range && (msg_ttl > 0)){
                    retransmit_data(send_socket_fd, config_list, kv_list, self_addr->location, port);
                }
                // All other cases
                else{
                    // Message not in range or expired
                    ;
                }
            }
        }        
    }

    return 0;
}
