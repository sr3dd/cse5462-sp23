#include "message.h"
#include "list.h"
#include "validator.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


struct list *parse_payload(char *buffer){
    struct list *kv_list = init_list(sizeof(struct pair));

    // check if received string contains the '^' character making it invalid
	if(strcspn(buffer, "^") < strlen(buffer)){
		printf("Invalid payload: string contains a '^' character\n");
		return NULL;
	}

	int count_messages = 0;

    // Replace spaces between quotes with ^ character
	for(unsigned int i=0, j=0; i<strlen(buffer); i++){
		if(buffer[i] == '\"'){
			++j;
			count_messages++;
			continue;
		}

		if(j%2 != 0){
			if(buffer[i] == ' '){
				buffer[i] = '^';
			}
		}
	}

    // Check if unbounded quote character exists
	if(count_messages  == 1){
		printf("Invalid payload: string contains a single unclosed quote\n");
		return NULL;
	}

    // Check if payload contains more than one msg key-value pairs - not allowed
	if(count_messages > 2){
		printf("Invalid payload: string contains more than one quoted value pairs\n");
		return NULL;
	}

	char sstr[100];
	char *token;
    // Tokenize string based on whitespace character
	token = strtok(buffer, " ");
	while(token != NULL){
		strcpy(sstr, token);
        // extract key-value pair from token
		struct pair *key_val = parse_kv(sstr);
		
        if(key_val){
            append(kv_list, key_val);
        }

		token = strtok(NULL, " ");
	}

	return kv_list;
}

struct pair *parse_kv(char* sstr){

    // Check if last character of token is : character
    // this means there is no value in given key-value token
    // invalid key-value pair
	if(strcspn(sstr, ":") == strlen(sstr)){
		printf("Valid format is <key>:<value> not %s\n", sstr);
		return NULL;
	}

    // Check if first character of token is : character
    // this means there is no key in given key-value token
    // invalid key-value pair
	if(strcspn(sstr, ":") == 0){
		printf("Valid key does not exist: %s\n", sstr);
		return NULL;
	}

    // get position of : character to break token into key and value pairs
	unsigned int kv_delim = strcspn(sstr, ":");
	
	char *key = malloc(sizeof(char)*(kv_delim+1));	
	strncpy(key, sstr, kv_delim);
	key[kv_delim] = '\0';

    char *val = malloc(sizeof(char)*(strlen(sstr)-kv_delim+1));
	strncpy(val, &sstr[kv_delim+1], strlen(sstr)-kv_delim);
    val[strlen(sstr)-kv_delim] = '\0';

    if(strcspn(val, "\"") < strlen(val) && (strcmp(key, "msg")) != 0){
	    printf("Invalid! A quoted value can only belong to a 'msg' key : %s\n", sstr);
        return NULL;
	}

    // if key is "msg", then replace ^ with whitespace characters again
	if((strcmp(key, "msg")) == 0){
		for(unsigned int i=0; i<strlen(val); i++){
			if(val[i] == '^'){
				val[i] = ' ';
			}
		}
	}

    struct pair *kv = malloc(sizeof(struct pair));
    kv->key = key;
    kv->value = val;

	return kv;
}

bool is_valid_message(struct list *kv_list, int port){

	if(!has_no_duplicates(kv_list)){
		printf("Duplicate keys exist.\n");
		return false;
	}

	if(!has_mandatory_keys(kv_list)){
		printf("Mandatory keys are missing.\n");
		return false;
	}

	if(!valid_version(kv_list, "4")){
		printf("'version' key value is not valid.\n");
		return false;
	}

	if(!valid_port(kv_list, port)){
		printf("'port' key value does not match server port.\n");
		return false;
	}

	return true;
}

void print_key_value_pairs(void *data){
	
	printf("\t%-20s", ((struct pair *)data)->key);
	printf("\t%-20s\n", ((struct pair *)data)->value);

}

char *append_location(const char *og_msg, int location){
    char append_str[15];
    sprintf(append_str, " location:%d", location);

    char *str = malloc(strlen(og_msg) + strlen(append_str));
	strcpy(str, og_msg);
	strcpy(str+strlen(og_msg), append_str);
    str[strlen(str)] = '\0';

    return str;
}

void print_payload(struct list *kv_list){
	printf("*****************************************************************\n");
	printf("\t%-20s\t%-20s\n", "Key", "Value");
	print(kv_list, print_key_value_pairs);
	printf("*****************************************************************\n");
}

bool is_key_match(void *node_data, void *data){
	struct pair *node = (struct pair *)node_data;
	char *key = (char *)data;

	if(strcmp(node->key, key) == 0){
		return true;
	}

	return false;
}
