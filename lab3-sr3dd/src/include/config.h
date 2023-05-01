#ifndef CONFIG_H
#define CONFIG_H

#include "list.h"
#include <stdlib.h>
#include <stdbool.h>

#define CONFIG_FILE_NAME "config.file"

// a struct to hold config info from each line of the config file
struct config{
    char* ip_addr;
    int port;
    int location;
};

// Reads a config file at given path and returns a list of config lines in file
struct list *read_config_file(char *file_path);
// constructs a config struct (ip, port, location) from a line read from the config file
struct config *parse_address_line(const char *config_line);
// Validates a line of config read from the config file
bool is_valid_config(struct config *);
// Returns true if port of the node data same as given port
bool match_by_port(void *, void *);

#endif
