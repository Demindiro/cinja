#include "../../include/temp/list.h"
#include <stdlib.h>
#include <string.h>
#include "temp-alloc.h"


cinja_list cinja_temp_list_create()
{
	cinja_list ls = temp_alloc(sizeof(*ls));
	if (ls == NULL)
		return NULL;
	ls->size  = 1024;
	ls->items = temp_alloc(ls->size * sizeof(*ls->items));
	if (ls->items == NULL)
		return NULL;
	ls->count = 0;
	return ls;
}


int _cinja_temp_list_add(cinja_list ls, void *item, enum cinja_type type)
{
	if (ls->size >= ls->count) {
		return -1; // TODO
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


int cinja_temp_list_remove(cinja_list ls, size_t index)
{
	if (ls->count <= index)
		return -1;
	ls->count--;
	memmove(ls->items +  index,
	        ls->items + (index + 1),
	        ls->count - index);
	return 0;
}
