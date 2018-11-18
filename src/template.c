#include "../include/template.h"
#include "../lib/string/include/string.h"
#include <stdio.h>
#include <stdlib.h>


#define FLAG_TYPE 0xF
#define FLAG_TYPE_SUBST 0x1
#define FLAG_TYPE_EXPR  0x2


static string nullstr;

__attribute__((constructor))
void init_nullstr() {
	nullstr = string_create("NULL");
}


/*
 * Helpers
 */
static int _skip_while(string str, size_t *i, const char *chars)
{
	while(1) {
		if (*i >= str->len)
			return 0;
		const char c = str->buf[*i];
		for (const char *d = chars; *d != 0; d++) {
			if (*d == c)
				goto cont;
		}
		return 1;
	cont:
		(*i)++;
	}
}


static int _skip_until(string str, size_t *i, const char *chars)
{
	while(1) {
		if (*i >= str->len)
			return 0;
		const char c = str->buf[*i];
		for (const char *d = chars; *d != 0; d++) {
			if (*d == c)
				return 1;
		}
		(*i)++;
	}
}



/*
 * Public
 */
cinja_template cinja_create_from_string(string str)
{
	cinja_template t = malloc(sizeof(*t));
	size_t s = 16;
	t->ptr = malloc(sizeof(*t->ptr) * s);
	t->flags = malloc(sizeof(*t->flags) * s);
	t->count = 0;

	size_t i = 0, start = i;
	while (i < str->len) {
		if (str->buf[i] == '{') {
			size_t end = i - 1;
			i++;
			if (i >= str->len)
				break;
			
			if (str->buf[i] == '{') {
				i++;
				if (!_skip_while(str, &i, " \t\n"))
					goto error;
				size_t s = i;
				if (!_skip_until(str, &i, " \t\n}"))
					goto error;

				string var = string_copy(str, s, i);
				if (var == NULL)
					goto error;
				t->vars[2*t->count + 1] = var;

				if (!_skip_while(str, &i, " \t\n"))
					goto error;
				if (i - 1 >= str->len || str->buf[i] != '}' || str->buf[i+1] != '}')
					goto error;

				t->flags[t->count] = FLAG_TYPE_SUBST;
			} else {
				goto skip;
			}
			t->text[2*t->count] = string_copy(str, start, end);
			start = i + 2;
			t->count++;
		skip:;
		}
		i++;
	}
	t->text[2*t->count] = string_copy(str, start, str->len);
	return t;
error:
	return NULL;
}


cinja_template cinja_create_from_file(const char *path)
{
	size_t len;
	FILE *f = fopen(path, "r");
	if (f == NULL)
		return NULL;

	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	string str = malloc(sizeof(str->len) + len);
	str->len = len;
	fread(str->buf, len, 1, f);
	str->buf[len] = 0;
	fclose(f);

	cinja_template t = cinja_create_from_string(str);
	free(str);
	return t;
}


/*
 * Returns a null-terminated string generated from a template
 */
string cinja_render(cinja_template temp, cinja_dict dict)
{
	string *strs = malloc(sizeof(*strs) * (2 * temp->count + 1));
	strs[0] = temp->text[0];
	for (size_t i = 0; i < temp->count; i++) {
		switch (temp->flags[i] & FLAG_TYPE) {
		case FLAG_TYPE_SUBST:;
			cinja_dict_entry_t entry = cinja_dict_get(dict, temp->vars[i*2 + 1]);
			strs[i*2 + 1] = entry.value == NULL ? nullstr : entry.value;
			break;
		case FLAG_TYPE_EXPR:
			break; // TODO
		default:
			break; // TODO
		}
	}
	strs[2*temp->count] = temp->text[2*temp->count];
	return string_concat(strs, 2 * temp->count + 1);
}
