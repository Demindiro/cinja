#include "../../include/temp/dict.h"
#include <errno.h>
#include <stdlib.h>
#include "temp-alloc.h"
#include "temp-cstring.h"


static size_t _get_index(cinja_dict d, string key)
{
	for (size_t i = 0; i < d->count; i++) {
		if (string_eq(d->entries[i].key, key))
			return i;
	}
	return -1;
}


static int _grow_dict(cinja_dict d)
{
	size_t s = d->size * 3 / 2;
	void *tmp = realloc(d->entries, sizeof(*d->entries) * s);
	if (tmp == NULL)
		return -1;
	d->entries = tmp;
	d->size = s;
	return 0;	
}


cinja_dict cinja_temp_dict_create()
{
	cinja_dict d = temp_alloc(sizeof(*d));
	d->count   = 0;
	d->size    = 1024;
	d->entries = temp_alloc(sizeof(*d->entries) * d->size);
	if (d->entries == NULL)
		return NULL;
	return d;
}


int _cinja_temp_dict_set(cinja_dict dict, string key, void *value, enum cinja_type type)
{
	size_t i = _get_index(dict, key);
	if (i == -1) {
		i = dict->count;
		if (dict->size <= dict->count && _grow_dict(dict) < 0)
			return -1;
		dict->count++;
		dict->entries[i].key = key;
	}
	dict->entries[i].value = value;
	dict->entries[i].type  = type;
	return 0;
}
