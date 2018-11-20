#ifndef __CINJA_TEMPLATE_H__
#define __CINJA_TEMPLATE_H__


#include <stddef.h>
#include "dict.h"


#define CINJA_MASK_TYPE    0xF
#define CINJA_TYPE_SUBST   0x1
#define CINJA_TYPE_COMMENT 0x2
#define CINJA_TYPE_IF      0x3
#define CINJA_TYPE_ELIF    0x4
#define CINJA_TYPE_ELSE    0x5
#define CINJA_TYPE_ENDIF   0x6
#if 0
#define CINJA_TYPE_FOR     0x7
#define CINJA_TYPE_ENDFOR  0x8
#endif

#define CINJA_EXPR_OP_EQ  0X1
#define CINJA_EXPR_OP_NEQ 0x2
#if 0
#define CINJA_EXPR_OP_LT  0x3
#define CINJA_EXPR_OP_GT  0x4
#define CINJA_EXPR_OP_LEQ 0x5
#define CINJA_EXPR_OP_GEQ 0x6
#endif


typedef struct cinja_expr_for {
	string var;
	string val;
} *cinja_expr_for;


typedef struct cinja_expr_if {
	string var;
	string val;
	unsigned int op : 3;
} *cinja_expr_if;


typedef struct cinja_template {
	size_t count;
	union {
		void **ptr;

		string *text;
	
		string *vars;
		cinja_expr_if  *expr_ifs;
		cinja_expr_for *expr_fors;
	};
	unsigned char *types;
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
