#include "validator.h"
#include "message.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool has_no_duplicates(struct list *kv_list){
    struct node *tmp = kv_list->head;

    while(tmp){
        char *key = ((struct pair *)tmp->data)->key;
        struct node *tmp_n = tmp->next;

        while(tmp_n){
            if(strcmp(key, ((struct pair *)tmp_n->data)->key) == 0){
                return false;
            }
            tmp_n = tmp_n->next;
        }
        tmp = tmp->next;
    }

    return true;
}

bool has_mandatory_keys(struct list *kv_list){
    bool version_present = false;
    bool port_present = false;
    bool location_present = false;

    struct node *tmp = kv_list->head;

    while(tmp){
        char *key = ((struct pair *)tmp->data)->key;

        if(strcmp(key, "version") == 0){
            version_present = true;
        }
        else if(strcmp(key, "port") == 0){
            port_present = true;
        }
        else if(strcmp(key, "location") == 0){
            location_present = true;
        }

        tmp = tmp->next;
    }

    return (version_present && port_present && location_present);
}

bool valid_version(struct list *kv_list, char *version){
    struct node *tmp = kv_list->head;

    while(tmp){
        char *key = ((struct pair *)tmp->data)->key;

        if(strcmp(key, "version") == 0){
            if(strcmp(((struct pair *)tmp->data)->value, version) == 0){
                return true;
            }
            else{
                printf("Message does not contain a valid version value.\n");
                return false;
            }
        }

        tmp = tmp->next;
    }

    //no "version" key found in key-value node list
    // this path will never execute since we check for 
    // mandatory keys before we do the version value check
    return false;
}

bool valid_port(struct list *kv_list, int port){
    struct node *tmp = kv_list->head;
    
    while(tmp){
        char *key = ((struct pair *)tmp->data)->key;

        if(strcmp(key, "port") == 0){
            int port_node = strtol(((struct pair *)tmp->data)->value, NULL, 10);
            
            if(port_node == port){
                return true;
            }
            else{
                printf("Message not addressed to this server.\n");
                return false;
            }
        }

        tmp = tmp->next;
    }

    // no "port" key found in key-value node list
    // this path will never execute since we check for 
    // mandatory keys before we do the port value check
    return false;

}
