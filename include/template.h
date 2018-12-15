#ifndef __CINJA_TEMPLATE_H__
#define __CINJA_TEMPLATE_H__


#include <stddef.h>
#include "dict.h"


enum cinja_expr_type {
	IF,
	ELIF,
	ELSE,
	END,
	FOR,
};

enum cinja_cmp {
	 EQ,
	NEQ,
};

typedef struct cinja_expr_arg {
	string val;
	int is_const : 1;
} cinja_expr_arg_t, *cinja_expr_arg;

typedef struct cinja_expr {
	enum cinja_expr_type type;
} cinja_expr_t, *cinja_expr;

typedef struct cinja_expr_if {
	enum cinja_expr_type type;
	cinja_expr_arg_t arg_l, arg_r;
	enum cinja_cmp cmp;
} cinja_expr_if_t, *cinja_expr_if;

typedef struct cinja_expr_for {
	enum cinja_expr_type type;
	string iterator;
	string var;
} cinja_expr_for_t, *cinja_expr_for;


typedef struct cinja_subst {
	size_t  varcount;
	size_t  funccount;
	string *vars;
	string *funcs;
} cinja_subst_t, *cinja_subst;


typedef struct cinja_template {
	size_t count;
	union {
		void       **ptr  ;
		string      *text ;
		cinja_subst *subst;
		cinja_expr  *expr ;
	};
	unsigned char *flags;
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
