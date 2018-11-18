#ifndef __CINJA_TEMPLATE_H__
#define __CINJA_TEMPLATE_H__


#include <stddef.h>
#include "dict.h"


typedef struct cinja_template {
	size_t count;
	union {
		void **ptr;
		string *vars;
		string *text;
		struct cinja_template **temps;
	};
	int *flags;
} cinja_template_t, *cinja_template;


/*
 * Create a new template from a string
 */
cinja_template cinja_create_from_string(string str);


/*
 * Create a new template from a file
 */
cinja_template cinja_create_from_file(const char *path);


/*
 * Frees all resources allocated by a template
 */
void cinja_free(cinja_template temp);


/*
 * Returns a null-terminated string generated from a template
 */
string cinja_render(cinja_template temp, cinja_dict dict);


#endif
