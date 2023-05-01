#include "validator.h"
#include "message.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool has_no_duplicate_keys(struct list *kv_list){
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

    for(int i = 0; i < (int)(sizeof(MANDATORY_KEYS)/sizeof(char *)); i++){
        struct node *tmp = kv_list->head;

        bool present = false;

        while(tmp){
        
            char *key = ((struct pair *)tmp->data)->key;

            if(strcmp(key, MANDATORY_KEYS[i]) == 0){
                present = true;
                break;
            }

            tmp = tmp->next;
        }

        if(!present){
            printf("Message does not contain all the mandatory keys.\n");
            return false;
        }
    }

    return true;
}

bool valid_version(struct list *kv_list, char *version){
    struct pair *version_kv = (struct pair *)find(kv_list, "version", is_key_match);

    if(strcmp(version_kv->value, version) == 0){
        return true;
    }
    else{
        printf("Message does not contain a valid version value.\n");
        return false;
    }

    //no "version" key found in key-value node list
    // this path will never execute since we check for 
    // mandatory keys before we do the version value check
    return false;
}

bool valid_port(struct list *kv_list, int port){
    struct pair *version_port = (struct pair *)find(kv_list, "toPort", is_key_match);

    if(atoi(version_port->value) == port){
        return true;
    }
    else{
        return false;
    }

    // no "port" key found in key-value node list
    // this path will never execute since we check for 
    // mandatory keys before we do the port value check
    return false;

}
