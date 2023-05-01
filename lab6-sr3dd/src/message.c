#include "message.h"
#include "list.h"
#include "validator.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


struct list *parse_payload(char const *buffer){
    struct list *kv_list = init_list(sizeof(struct pair));

	char *buffer_cpy = malloc(strlen(buffer));
	strcpy(buffer_cpy, buffer);

    // check if received string contains the '^' character making it invalid
	if(strcspn(buffer_cpy, "^") < strlen(buffer_cpy)){
		printf("Invalid payload: string contains a '^' character\n");
		return NULL;
	}

	int count_messages = 0;

    // Replace spaces between quotes with ^ character
	for(unsigned int i=0, j=0; i<strlen(buffer_cpy); i++){
		if(buffer_cpy[i] == '\"'){
			++j;
			count_messages++;
			continue;
		}

		if(j%2 != 0){
			if(buffer_cpy[i] == ' '){
				buffer_cpy[i] = '^';
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
	token = strtok(buffer_cpy, " ");
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

bool is_valid_message(struct list *kv_list){

	if(!has_no_duplicate_keys(kv_list)){
		printf("Duplicate keys exist.\n");
		return false;
	}

	if(!has_mandatory_keys(kv_list)){
		printf("Mandatory keys are missing.\n");
		return false;
	}

	if(!valid_version(kv_list, "6")){
		printf("'version' key value is not valid.\n");
		return false;
	}

	return true;
}

void print_key_value_pairs(void *data){
	
	printf("\t%-20s", ((struct pair *)data)->key);
	printf("\t%-20s\n", ((struct pair *)data)->value);

}

char *message_constructor(const char *og_msg, int seqNumber, 
								int location, int port, int ttl){
    char append_str[100];
    sprintf(append_str, " seqNumber:%d location:%d fromPort:%d TTL:%d send-path:%d", 
						seqNumber, location, port, ttl, port);

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

char *construct_str_message(struct list *kv_list){
	char *msg = NULL;
	struct node *tmp = kv_list->head;

	while(tmp){
		char kv_str[200];
		struct pair *kv = (struct pair *)tmp->data;
		sprintf(kv_str, "%s:%s ", kv->key, kv->value);
		char *extend = realloc(msg, (msg?strlen(msg):0) + strlen(kv_str));
		
		if(extend){
			msg = extend;
		}
		else{
			printf("Realloc failed\n");
			exit(1);
		}

		strcat(msg, kv_str);
		tmp = tmp->next;
	}

	if(msg){
		msg[strlen(msg)-1] = '\0';
	}

	return msg;
}

bool is_port_match(void *node_data, void *data){
	struct history *node = (struct history *)node_data;
	int port = *(int *)data;

	if(node->port == port){
		return true;
	}

	return false;
}

bool is_seq_match(void *node_data, void *data){
	struct seq_node *node = (struct seq_node *)node_data;
	int seq = *(int *)data;

	if (node->seqNum == seq){
		return true;
	}

	return false;
}

bool is_ack(struct list *kv_list){
	struct pair *type_kv = (struct pair *)find(kv_list, "type", is_key_match);

	return type_kv ? strcmp(type_kv->value, "ACK") == 0 : false;
}

bool is_duplicate(struct list *kv_list, struct list *history_list){
	int from_port_kv = atoi(((struct pair *)find(kv_list, "fromPort", is_key_match))->value);
	int seq_num_kv = atoi(((struct pair *)find(kv_list, "seqNumber", is_key_match))->value);

	struct history *port_record = 
            (struct history *)find(history_list, 
					&from_port_kv, is_port_match);

	if(port_record){
		struct seq_node *seq = (struct seq_node *)find(port_record->seq_list, 
								&seq_num_kv, is_seq_match);
		return seq ? true : false;
	}
	else{
		return false;
	}
}

bool is_duplicate_ack(struct list *kv_list, struct list *history_list){
	int from_port_kv = atoi(((struct pair *)find(kv_list, "fromPort", is_key_match))->value);
	int seq_num_kv = atoi(((struct pair *)find(kv_list, "seqNumber", is_key_match))->value);

	struct history *port_record = 
            (struct history *)find(history_list, 
					&from_port_kv, is_port_match);

	if(port_record){
		struct seq_node *seq = (struct seq_node *)find(port_record->seq_list, 
								&seq_num_kv, is_seq_match);
		return seq ? seq->acked : false;
	}
	else{
		return false;
	}
}

void update_history(struct list *kv_list, struct list *history_list, enum message_type msg){
	int kv_from_port = atoi(((struct pair *)find(kv_list, "fromPort", is_key_match))->value);
	int kv_seq_num = atoi(((struct pair *)find(kv_list, "seqNumber", is_key_match))->value);

	struct history *port_record = 
            (struct history *)find(history_list, 
					&kv_from_port, is_port_match);

	if(msg == REG){
		struct seq_node *new_seq = malloc(sizeof(struct seq_node));
    	new_seq->seqNum = kv_seq_num;
    	new_seq->acked = false;

		if(!port_record){
			struct history *new_port_record = malloc(sizeof(struct history));
			new_port_record->port = kv_from_port;
			new_port_record->seq_list = init_list(sizeof(struct seq_node));
			append(new_port_record->seq_list, new_seq);
			append(history_list, new_port_record);
		}
		else{
			append(port_record->seq_list, new_seq);
		}
		
		return;
	}
	else if(msg == ACK){
		struct seq_node *out_seq_node = 
			(struct seq_node *)find(port_record->seq_list, 
				&kv_seq_num, is_seq_match);
		
		out_seq_node->acked = true;
		return;
	}
}
