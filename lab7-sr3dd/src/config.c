#define  _GNU_SOURCE // to suppress GCC compilation warnings
#include "config.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// some private function declarations

bool is_valid_ip_addr(const char *);
bool is_valid_port_val(int);
bool is_valid_location_val(int);

bool is_valid_config(struct config *c){
    
    if(is_valid_ip_addr(c->ip_addr) &&
            is_valid_port_val(c->port)  &&
            is_valid_location_val(c->location)){
        return true;
    }
    else{
        return false;
    }
}

bool match_by_port(void *node_data, void *port){
    int node_port = ((struct config *)node_data)->port;
    int match_port = *(int *)port;

    if(node_port == match_port){
        return true;
    }
    else{
        return false;
    }
}

struct list *read_config_file(char *file_path){
    struct list *config_list = init_list(sizeof(struct config));

    // file descriptor
    FILE *fp;

    // Attempt to open file corresponding to given filename
    if((fp = fopen(file_path, "r")) == NULL){
        printf("Could not open file: %s\n", file_path);
        return NULL;
    }

    char *line_str = NULL;
    size_t line_len = 0;
    ssize_t read_len;

    while((read_len = getline(&line_str, &line_len, fp)) != -1){
    // allocate a string of read line length
        char *line = malloc(sizeof(char)*line_len);
        // remove the '\n' at the end of line input if it exists
        line_str[strcspn(line_str, "\r\n")] = '\0';
        // copy into line so line_str can be reused for next read
        strcpy(line, line_str);
        struct config *row = parse_address_line(line);
        
        if(is_valid_config(row)){
            append(config_list, row);
        }
    }

    return config_list;
}

struct config *parse_address_line(const char *config_line){
    char *token_line = malloc(sizeof(char)*strlen(config_line));
    strcpy(token_line, config_line);
    char *token;
  
    // Tokenize string based on whitespace character
	token = strtok(token_line, " ");
    int i = 0;

    struct config *c = malloc(sizeof(struct config));

	while(token != NULL){
        if(i == 0){
            c->ip_addr = malloc(sizeof(char)*strlen(token));
            strcpy(c->ip_addr, token);
        }
        else if(i == 1){
            c->port = atoi(token); 
        }
        else if(i == 2){
            c->location = atoi(token);
        }
        i++;
        token = strtok(NULL, " ");
	}

    return c;
}

// private function definitions

bool is_valid_ip_addr(const char *ip_str){
    struct sockaddr_in inaddr;
 
    if (!inet_pton(AF_INET, ip_str, &inaddr)){
        return false;
    }
    else{
        return true;
    }
}

bool is_valid_port_val(int port){
    if ((port > 65535) || (port < 0)){
        return false;
    }
    else{
        return true;
    }
}

bool is_valid_location_val(int location){
    if ((location > 50) || (location < 0)){
        return false;
    }
    else{
        return true;
    }
}
