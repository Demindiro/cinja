#include "../include/dict.h"
#include "../lib/string/include/string.h"
#include <errno.h>
#include <stdlib.h>


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


cinja_dict cinja_dict_create()
{
	cinja_dict d = malloc(sizeof(*d));
	d->count   = 0;
	d->size    = 4;
	d->entries = malloc(sizeof(*d->entries) * d->size);
	if (d->entries == NULL) {
		free(d);
		return NULL;
	}
	return d;
}


void cinja_dict_free(cinja_dict d)
{
	for (size_t i = 0; i < d->count; i++) {
		free(d->entries[i].value);
		free(d->entries[i].key);
	}
	free(d->entries);
	free(d);
}


int _cinja_dict_set(cinja_dict dict, string key, void *value, int type)
{
	if (type < 0 || CINJA_DICT_TYPE_MAX < type) {
		errno = EINVAL;
		return -1;
	}
	size_t i = _get_index(dict, key);
	if (i == -1) {
		i = dict->count;
		if (dict->size <= dict->count && _grow_dict(dict) < 0)
			return -1;
		dict->count++;
	}
	dict->entries[i].key   = key;
	dict->entries[i].value = value;
	dict->entries[i].type  = type;
	return 0;
}


cinja_dict_entry_t cinja_dict_get(cinja_dict dict, string key)
{
	cinja_dict_entry_t entry;
	size_t i = _get_index(dict, key);
	if (i == -1) {
		entry.value = NULL;
		return entry;
	}
	return dict->entries[i];
}


cinja_dict_entry_t cinja_dict_iter(cinja_dict d, void **state)
{
	size_t *i;
	if (*state == NULL)
		*state = calloc(sizeof(*i), 1);
	i = *state;
	if (*i >= d->count) {
		free(*state);
		*state = NULL;
		cinja_dict_entry_t e = { 0 };
		return e;
	}
	cinja_dict_entry_t e = d->entries[*i];
	(*i)++;
	return e;
}
