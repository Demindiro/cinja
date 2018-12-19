#ifndef CINJA_TEMP_LIST_H__
#define CINJA_TEMP_LIST_H__

#include <stddef.h>
#include "../list.h"
#include "types.h"

cinja_list cinja_temp_list_create();

int _cinja_temp_list_add(cinja_list ls, void *item, enum cinja_type type);
#define cinja_temp_list_add(ls, value) _cinja_list_add(ls, value, GET_TYPE(value))

int cinja_temp_list_remove(cinja_list ls, size_t index);

#endif
