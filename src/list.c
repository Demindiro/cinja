#include "../include/list.h"
#include <stdlib.h>
#include <string.h>

cinja_list cinja_list_create()
{
	cinja_list ls = malloc(sizeof(*ls));
	if (ls == NULL)
		return NULL;
	ls->size  = 2;
	ls->items = malloc(ls->size * sizeof(*ls->items));
	if (ls->items == NULL) {
		free(ls);
		return NULL;
	}
	ls->count = 0;
	return ls;
}


void cinja_list_free(cinja_list ls)
{
	free(ls->items);
	free(ls);
}


int _cinja_list_add(cinja_list ls, void *item, enum cinja_type type)
{
	if (ls->size == ls->count) {
		size_t s  = (ls->size * 3) / 2;
		void *tmp = realloc(ls->items, s * sizeof(*ls->items));
		if (tmp == NULL)
			return -1;
		ls->items = tmp;
		ls->size  = s;
	}
	ls->items[ls->count].item = item;
	ls->items[ls->count].type = type;
	ls->count++;
	return 0;
}


int cinja_list_remove(cinja_list ls, size_t index)
{
	if (ls->count <= index)
		return -1;
	ls->count--;
	memmove(ls->items +  index,
	        ls->items + (index + 1),
	        ls->count - index);
	return 0;
}


int _cinja_list_set(cinja_list ls, size_t i, void *item, enum cinja_type type)
{
	if (i > ls->count)
		return -1;
	ls->items[i].item = item;
	ls->items[i].type = type;
	return 0;
}


cinja_list_entry_t cinja_list_get(cinja_list ls, size_t i)
{
	cinja_list_entry_t inval = { 0 };
	if (i > ls->count)
		return inval;
	return ls->items[i];
}
