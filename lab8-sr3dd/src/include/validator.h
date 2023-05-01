#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "list.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MANDATORY_KEYS (char  const *[]){"version", "fromPort", "toPort", "location", "TTL", "msg", "send-path"}
#define MOVE_MANDATORY_KEYS (char const *[]){"version", "fromPort", "toPort", "move"}

// check there are no duplicate keys in parsed message
bool has_no_duplicate_keys(struct list *);
// check all mandatory keys for a REG type message
bool has_mandatory_keys(struct list *);
// check all mandatory keys for a MOV type message
bool has_move_mandatory_keys(struct list *);
// check value of the 'version' key is as expected
// eg. lab3 -> version:3
bool valid_version(struct list *, char *);
// check value of the 'port' key is same as port of the drone server
bool valid_port(struct list *, int);

#endif
