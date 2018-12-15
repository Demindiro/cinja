#include "../include/template.h"
#include "../lib/string/include/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/dict.h"
#include "../include/list.h"


#define CINJA_TYPE         0xF
#define CINJA_TYPE_SUBST   0x1
#define CINJA_TYPE_EXPR    0x2
#define CINJA_TYPE_COMMENT 0x3

static string nullstr;


__attribute__((constructor))
static void init()
{
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


static cinja_expr _parse_expr_if(string str, enum cinja_expr_type type)
{
	size_t        i = 0;
	cinja_expr_if e = malloc(sizeof(*e));

	size_t j = i;
	const char *s = str->buf[i] == '"' ? "\"" : " \t\n!=";
	if (!_skip_until(str, &i, s))
		return NULL;
	e->arg_l.val = string_copy(str, j, i);
	e->arg_l.is_const = s[0] == '"';
	if (!e->arg_l.is_const && string_eq(e->arg_l.val, "none")) {
		free(e->arg_l.val);
		e->arg_l.val = NULL;
	}

	if (!_skip_while(str, &i, " \t\n"))
		return NULL;
	j = i;
	if (!_skip_until(str, &i, " \t\n\""))
		return NULL;
	string cmp = string_copy(str, j, i);
	if (string_eq(cmp, "=="))
		e->cmp = EQ;
	else if (string_eq(cmp, "!="))
		e->cmp = NEQ;
	else
		return NULL;
	free(cmp);
	if (!_skip_while(str, &i, " \t\n"))
		return NULL;

	j = i;
	s = str->buf[i] == '"' ? "\"" : " \t\n";
	if (!_skip_until(str, &i, s))
		return NULL;
	e->arg_r.val = string_copy(str, j, i);
	e->arg_r.is_const = s[0] == '"';
	if (!e->arg_l.is_const && string_eq(e->arg_l.val, "none")) {
		free(e->arg_l.val);
		e->arg_l.val = NULL;
	}

	e->type = type;
	return (cinja_expr)e;
}


static cinja_expr _parse_expr_for(string str, enum cinja_expr_type type)
{
	cinja_expr_for e = malloc(sizeof(*e));
	size_t i = 0;
	
	if (!_skip_until(str, &i, " \t\n"))
		return NULL;
	e->iterator = string_copy(str, 0, i);

	if (!_skip_while(str, &i, " \t\n"))
		return NULL;
	size_t j = i;
	if (!_skip_until(str, &i, " \t\n"))
		return NULL;
	string op = string_copy(str, j, i);
	if (string_eq(op, "in")) {
		if (!_skip_while(str, &i, " \t\n"))
			return NULL;
		size_t j = i;
		_skip_until(str, &i, " \t\n");
		e->var = string_copy(str, j, i);
	} else {
		return NULL;
	}
	free(op);

	e->type = type;
	return (cinja_expr)e;
}


static cinja_expr _parse_expr(string str, size_t start, size_t end)
{
	size_t i = start;
	if (!_skip_while(str, &i, " \t\n"))
		return NULL;

	size_t j = i;
	if (!_skip_until(str, &i, " \t\n"))
		return NULL;
	string type = string_copy(str, j, i);

	if (!_skip_while(str, &i, " \t\n"))
		return NULL;
	string stre = string_copy(str, i, end);

	cinja_expr e;
	if (string_eq(type, "if")) {
		e = _parse_expr_if(stre, IF);
	} else if (string_eq(type, "elif")) {
		e = _parse_expr_if(stre, ELIF);
	} else if (string_eq(type, "else")) {
		e = malloc(sizeof(*e));
		e->type = ELSE;
	} else if (string_eq(type, "end")) {
		e = malloc(sizeof(*e));
		e->type = END;
	} else if (string_eq(type, "for")) {
		e = _parse_expr_for(stre, FOR);
	} else {
		e = NULL;
	}
	free(type);
	free(stre);
	return e;
}


/*
 * Public
 */
cinja_template cinja_create(string str)
{
	cinja_template t = malloc(sizeof(*t));
	size_t s = 128;
	t->ptr = malloc(sizeof(*t->ptr) * s * 2 + 1);
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
				size_t j = i;
				if (!_skip_until(str, &i, " \t\n(}"))
					goto error;

				cinja_subst s = malloc(sizeof(*s));
				if (str->buf[i] == '(') {
					string funcs = string_copy(str, j, i);
					s->funcs = string_split(funcs, '.', &s->funccount);
					free(funcs);
					i++;
					if (!_skip_while(str, &i, " \t\n"))
						goto error;
					j = i;
					if (!_skip_until(str, &i, " \t\n)"))
						goto error;
					string vars = string_copy(str, j, i);
					s->vars = string_split(vars, '.', &s->varcount);
					free(vars);
					if (!_skip_until(str, &i, ")"))
						goto error;
					i++;
				} else {
					s->funcs = NULL;
					s->funccount = 0;
					string vars = string_copy(str, j, i);
					s->vars = string_split(vars, '.', &s->varcount);
					free(vars);
					if (s->vars == NULL)
						goto error;
				}

				t->subst[2*t->count + 1] = s;
				if (!_skip_while(str, &i, " \t\n"))
					goto error;
				if (i - 1 >= str->len || str->buf[i] != '}' || str->buf[i+1] != '}')
					goto error;

				t->flags[t->count] = CINJA_TYPE_SUBST;
			} else if (str->buf[i] == '%') {
				i++;
				size_t j = i;
				if (!_skip_until(str, &i, "%"))
					goto error;
				cinja_expr e = _parse_expr(str, j, i);
				if (e == NULL)
					goto error;
				t->expr[2*t->count + 1] = e;

				t->flags[t->count] = CINJA_TYPE_EXPR;
			} else {
				goto skip;
			}
			t->text[2*t->count] = string_copy(str, start, end + 1);
			start = i + 2;
			t->count++;
		skip:;
		} else {
			i++;
		}
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
	
	string str = malloc(sizeof(str->len) + len + 1);
	str->len = len;
	fread(str->buf, len, 1, f);
	str->buf[len] = 0;
	fclose(f);

	cinja_template t = cinja_create(str);
	free(str);
	return t;
}


void cinja_free(cinja_template temp)
{
	for (size_t i = 0; i < temp->count; i++) {
		free(temp->text[i*2]);
		if (temp->flags[i] & CINJA_TYPE_EXPR) {
			enum cinja_expr_type t = temp->expr[i*2 + 1]->type;
			if (t == IF) {
				cinja_expr_if e = (cinja_expr_if)temp->expr[i*2 + 1];
				free(e->arg_l.val);
				free(e->arg_r.val);
			} else if (t == FOR) {
				cinja_expr_for e = (cinja_expr_for)temp->expr[i*2 + 1];
				free(e->iterator);
				free(e->var);
			}
		} else {
			cinja_subst s = temp->subst[i*2 + 1];
			for (size_t k = 0; k < s->varcount ; k++)
				free(s->vars [k]);
			for (size_t k = 0; k < s->funccount; k++)
				free(s->funcs[k]);
			free(s->funcs);
			free(s->vars );
		}
		free(temp->ptr[i*2 + 1]);
	}
	free(temp->text[temp->count*2]);
	free(temp->ptr);
	free(temp->flags);
	free(temp);
}



static void _cinja_render_skip_scope(cinja_template temp, size_t *index)
{
	ssize_t count = 0;
	 size_t     i = *index;
	while (count >= 0) {
		i++;
		if ((temp->flags[i] & CINJA_TYPE) == CINJA_TYPE_EXPR) {
			enum cinja_expr_type t = temp->expr[i*2 + 1]->type;
	 		switch (t) {
			case END:
				count--;
				break;
			case IF:
			case FOR:
				count++;
				break;
			case ELIF:
			case ELSE:
				break;
			}
		}
	}
	*index = i;
}


static cinja_dict_entry_t _cinja_render_get_var(string *vars, size_t count, cinja_dict dict)
{
	cinja_dict_entry_t e = cinja_dict_get(dict, vars[0]);
	for (size_t i = 1; i < count; i++) {
		if (e.value == NULL || e.type != DICT) {
			e.value = NULL;
			return e;
		}
		e = cinja_dict_get(e.value, vars[i]);
	}
	return e;
}


string cinja_render(cinja_template temp, cinja_dict dict)
{
	string *strs = malloc(1024 * sizeof(*strs));
	size_t count = 0;
	int  skip_else      [16] = { 0 };
	int   for_loop      [16] = { 0 };
	int   for_loop_start[16] = { 0 };
	int   for_loop_end  [16] = { 0 };
	int   for_loop_index[16] = { 0 };
	size_t scope_i           =  -1  ;
	cinja_dict locals        = cinja_dict_create();
	for (size_t i = 0; i < temp->count; i++) {
		strs[count++] = temp->text[2*i];
		switch (temp->flags[i] & CINJA_TYPE) {
		case CINJA_TYPE_SUBST:;
			cinja_subst s = temp->subst[i*2 + 1];
			cinja_dict_entry_t var = _cinja_render_get_var(s->vars, s->varcount, locals);
			if (var.value == NULL)
				var = _cinja_render_get_var(s->vars, s->varcount, dict);
			if (s->funcs == NULL) {
				if (var.value != NULL && var.type != STRING)
					return NULL;
				strs[count++] = var.value == NULL ? nullstr : var.value;
			} else {
				if (var.value == NULL || var.type != DICT)
					return NULL;
				cinja_dict_entry_t func = _cinja_render_get_var(s->funcs, s->funccount, dict);
				if (func.type != TEMPLATE)
					return NULL;
				string render = cinja_render(func.value, var.value);
				if (render == NULL)
					return NULL;
				strs[count++] = render;
			}
			break;
		case CINJA_TYPE_EXPR:
			switch (temp->expr[i*2 + 1]->type) {
			case ELIF:
				if (skip_else[scope_i]) {
					_cinja_render_skip_scope(temp, &i);
					break;
				}
				scope_i--;
			case IF: {
				scope_i++;
				cinja_expr_if e = (cinja_expr_if)temp->expr[i*2 + 1];
				string val_l = e->arg_l.is_const ? e->arg_l.val : cinja_dict_get(dict, e->arg_l.val).value;
				string val_r = e->arg_r.is_const ? e->arg_r.val : cinja_dict_get(dict, e->arg_r.val).value;
				enum cinja_cmp cmp;
				if (val_l == NULL && val_r == NULL)
					cmp = EQ;
				else if ((val_l == NULL && val_r != NULL) || (val_l != NULL && val_r == NULL))
					cmp = NEQ;
				else if (string_eq(val_l, val_r))
					cmp = EQ;
				else
					cmp = NEQ;
				if (cmp == e->cmp) {
					skip_else[scope_i] = 1;
				} else {
					skip_else[scope_i] = 0;
					_cinja_render_skip_scope(temp, &i);
				}
			}
			break;
			case END:
				if (for_loop[scope_i]) {
					for_loop_index[scope_i]++;
					i = for_loop_start[scope_i] - 1;
				}
				scope_i--;
				break;
			case ELSE:
				if (skip_else[scope_i]) {
					_cinja_render_skip_scope(temp, &i);
				}
				break;
			case FOR:
				scope_i++;
				if (!for_loop[scope_i]) {
					for_loop      [scope_i] = 1;
					for_loop_start[scope_i] = i;
					size_t j = i;
					_cinja_render_skip_scope(temp, &j);
					for_loop_end  [scope_i] = j - 1;
					for_loop_index[scope_i] = 0;
				} else {
					count--;
				}
				cinja_expr_for     e = (cinja_expr_for)temp->expr[i*2 + 1];
				cinja_dict_entry_t f = cinja_dict_get(dict, e->var);
				if (f.type != LIST)
					return NULL;
				cinja_list l = f.value;
				if (for_loop_index[scope_i] >= l->count) {
					for_loop[scope_i] = 0;
					i = for_loop_end[scope_i];
				} else {
					cinja_list_entry_t item = cinja_list_get(l, for_loop_index[scope_i]);
					_cinja_dict_set(locals, string_copy(e->iterator), item.item, item.type);
				}
				break;
			}
			break;
		default:
			return NULL; // TODO
		}
	}
	strs[count++] = temp->text[2*temp->count];
	string render = string_concat(strs, count);
	free(strs);
	cinja_dict_free(locals);
	return render;
}
