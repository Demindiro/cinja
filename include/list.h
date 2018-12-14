#ifndef CINJA_LIST_H__
#define CINJA_LIST_H__

#include <stddef.h>
#include "types.h"

typedef struct cinja_list_entry {
	void *item;
	enum cinja_type type;
} cinja_list_entry_t, *cinja_list_entry;

typedef struct cinja_list {
	size_t count;
	size_t size;
	cinja_list_entry_t *items;
} cinja_list_t, *cinja_list;


cinja_list cinja_list_create();

void cinja_list_free(cinja_list ls);

int _cinja_list_add(cinja_list ls, void *item, enum cinja_type type);
#define cinja_list_add(ls, value) _cinja_list_add(ls, value, GET_TYPE(value))

int cinja_list_remove(cinja_list ls, size_t index);

int _cinja_list_set(cinja_list ls, size_t i, void *item, enum cinja_type type);
#define cinja_list_set(ls, i, value) _cinja_list_set(ls, i, value, GET_TYPE(value))

cinja_list_entry_t cinja_list_get(cinja_list ls, size_t i);

#endif
