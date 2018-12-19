#ifndef CINJA_TEMP_DICT_H__
#define CINJA_TEMP_DICT_H__


#include "temp-cstring.h"
#include "../dict.h"
#include "list.h"
#include "types.h"
#include "template.h"


/*
 * Creates a new temporary dictionary. Temporary dictionaries don't have to be freed.
 */
cinja_dict cinja_temp_dict_create();


/*
 * Sets the value with the given key.
 * Existing values are removed *without* `free`
 * NULL effectively removes the value for the given key.
 */
int _cinja_temp_dict_set(cinja_dict dict, string key, void *value, enum cinja_type type);
#define cinja_temp_dict_set(dict, key, value) _cinja_dict_set(dict, key, value, GET_TYPE(value))


#endif
