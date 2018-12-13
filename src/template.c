#include "../include/template.h"
#include "../lib/string/include/string.h"
#include <stdio.h>
#include <stdlib.h>


#define FLAG_TYPE       0xF
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


static cinja_expr _parse_expr_if(string str, enum cinja_expr_type type)
{
	size_t        i = 0;
	cinja_expr_if e = malloc(sizeof(*e));

	if (!_skip_while(str, &i, " \t\n"))
		return NULL;

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


static cinja_expr _parse_expr(string str, size_t start, size_t end)
{
	size_t i = start;
	if (!_skip_while(str, &i, " \t\n"))
		return NULL;
	size_t j = i;
	if (!_skip_until(str, &i, " \t\n"))
		return NULL;
	string type = string_copy(str, j, i);
	cinja_expr e;
	string stre = string_copy(str, i, end);
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
		e = NULL;
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
	size_t s = 16;
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
			} else if (str->buf[i] == '%') {
				i++;
				size_t j = i;
				if (!_skip_until(str, &i, "%"))
					goto error;
				cinja_expr e = _parse_expr(str, j, i);
				if (e == NULL)
					goto error;
				t->expr[2*t->count + 1] = e;

				t->flags[t->count] = FLAG_TYPE_EXPR;
			} else {
				goto skip;
			}
			t->text[2*t->count] = string_copy(str, start, end + 1);
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
		switch (temp->flags[i] & FLAG_TYPE) {
		case FLAG_TYPE_SUBST:
			free(temp->vars[i*2 + 1]);
			break;
		case FLAG_TYPE_EXPR:
			/* TODO */
			break;
		}
	}
	free(temp->text[temp->count*2]);
	free(temp->ptr);
	free(temp->flags);
	free(temp);
}


string cinja_render(cinja_template temp, cinja_dict dict)
{
	string *strs = malloc(sizeof(*strs) * (2 * temp->count + 1));
	strs[0] = temp->text[0];
	size_t count = 0;
	int skip_else[16] = { 0 };
	int skip_else_i   =  -1  ;
	for (size_t i = 0; i < temp->count; i++) {
		strs[count++] = temp->text[2*i];
		switch (temp->flags[i] & FLAG_TYPE) {
		case FLAG_TYPE_SUBST:;
			cinja_dict_entry_t entry = cinja_dict_get(dict, temp->vars[i*2 + 1]);
			strs[count++] = entry.value == NULL ? nullstr : entry.value;
			break;
		case FLAG_TYPE_EXPR:
			switch (temp->expr[i*2 + 1]->type) {
			case ELIF:
				if (skip_else[skip_else_i])
					break;
			case IF: {
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
					skip_else[++skip_else_i] = 1;
				} else {
					skip_else[++skip_else_i] = 0;
					while (((temp->flags[i] & FLAG_TYPE) != FLAG_TYPE_EXPR) ||
					       (temp->expr[i*2 + 1]->type != END))
						i++;
				}
			}
			case END:
				skip_else_i--;
				break;
			case ELSE:
				if (skip_else[skip_else_i]) {
					while (((temp->flags[i] & FLAG_TYPE) != FLAG_TYPE_EXPR) ||
					       (temp->expr[i*2 + 1]->type != END))
						i++;
				}
				break;
			case FOR:
				break; // TODO
			}
			break;
		default:
			return NULL; // TODO
		}
	}
	strs[count++] = temp->text[2*temp->count];
	string render = string_concat(strs, count);
	free(strs);
	return render;
}
