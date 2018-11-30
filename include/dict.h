#ifndef CINJA_DICT_H__
#define CINJA_DICT_H__


#include "../lib/string/include/string.h"


#define CINJA_DICT_TYPE        0xF
#define CINJA_DICT_TYPE_STRING 0x0
#define CINJA_DICT_TYPE_DICT   0x1
#define CINJA_DICT_TYPE_MAX    0x1


typedef struct cinja_dict_entry {
	string  key;
	void   *value;
	int     type;
} cinja_dict_entry_t;


typedef struct cinja_dict {
	size_t              size;
	size_t              count;
	cinja_dict_entry_t *entries;
} cinja_dict_t, *cinja_dict;


/*
 * Creates a new dictionary.
 */
cinja_dict cinja_dict_create();


/*
 * Frees the resources used by a dictionary.
 * Both the keys and the values are freed.
 */
void cinja_dict_free(cinja_dict dict);


/*
 * Sets the value with the given key.
 * Existing values are removed *without* `free`
 * NULL effectively removes the value for the given key.
 */
int _cinja_dict_set(cinja_dict dict, string key, void *value, int type);
#define cinja_dict_set(dict, key, value) _cinja_dict_set(dict, key, value, \
	_Generic((value), string    : CINJA_DICT_TYPE_STRING, \
	                  cinja_dict: CINJA_DICT_TYPE_DICT))

/*
 * Returns the value specified by the key or `{ .value = NULL }`
 */
cinja_dict_entry_t cinja_dict_get(cinja_dict dict, string key);


/*
 * Iterates over all values of a dictionary
 */
cinja_dict_entry_t cinja_dict_iter(cinja_dict dict, void **state);


#endif
