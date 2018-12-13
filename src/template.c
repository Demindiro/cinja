#include "../include/template.h"
#include "../lib/string/include/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static string nullstr;
static string emptystr;
static string op_eq;
static string op_neq;
#if 0
static string op_lt;
static string op_gt;
static string op_leq;
static string op_geq;
static string op_is;
#endif


__attribute__((constructor))
void init_static_strs() {
	nullstr = string_create("");
	emptystr = nullstr;
	op_eq   = string_create("==");
	op_neq  = string_create("!=");
#if 0
	op_lt   = string_create("<" );
	op_gt   = string_create(">" );
	op_leq  = string_create("<=");
	op_geq  = string_create(">=");
	op_is   = string_create("is");
#endif
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
 * Expressions 
 */
static cinja_expr_if parse_if(string str, size_t *i)
{
	cinja_expr_if e = calloc(sizeof(*e), 1);

	// var	
	if (!_skip_while(str, i, " \t\n"))
		goto error;
	size_t start = *i;
	if (!_skip_until(str, i, " \t\n=!<>"))
		goto error;
	e->var = string_copy(str, start, *i);
	if (e->var == NULL || !_skip_while(str, i, " \t\n"))
		goto error;

	// op
	char c = str->buf[*i];
	(*i)++;
	switch (c) {
	case '=':
	case '!':
		if (str->buf[*i] != '=')
			goto error;
		e->op = c == '=' ? CINJA_EXPR_OP_EQ : CINJA_EXPR_OP_NEQ;
		(*i)++;
		break;
#if 0
	case '<':
		if (str->buf[*i] == '=') {
			 e->op = CINJA_EXPR_OP_LEQ;
			(*i)++;
		} else {
			e->op = CINJA_EXPR_OP_LT;
		}
		break;
	case '>':
		if (str->buf[*i] == '=') {
			 e->op = CINJA_EXPR_OP_GEQ;
			(*i)++;
		} else {
			e->op = CINJA_EXPR_OP_GT;
		}
		break;
#endif
	default:
		goto error;
	}

	// val
	if (!_skip_while(str, i, " \t\n"))
		goto error;
	if (str->buf[*i] == '"') {
		size_t start = *i + 1;
		do {
			(*i)++;
			if (str->buf[*i] == 0)
				goto error;
		} while (str->buf[*i] != '"');
		e->val = string_copy(str, start, *i);
		(*i)++;
	} else if (strncmp(&str->buf[*i], "none", 4) == 0) {
		*i += 4;
		char c = str->buf[*i];
		if (c != ' ' && c != '\t' && c != '\n')
			goto error;
		e->val = NULL;
	} else {
		goto error;
	}

	return e;
error:
	free(e->var);
	free(e->val);
	free(e);
	return NULL;
}



/*
 * Public
 */
cinja_template cinja_create_from_string(string str)
{
	cinja_template t = malloc(sizeof(*t));
	size_t s = 128;
	t->ptr = malloc(sizeof(*t->ptr) * s);
	t->types = malloc(sizeof(*t->types) * s);
	t->count = 0;

	int    type_stack[16];
	size_t type_stack_i = 0;
	size_t i = 0, start = i;
	int trim_next = 0;
	while (i < str->len) {
		if (str->buf[i] == '{') {
			size_t end = i;
			i++;
			if (i >= str->len)
				break;
			char c = str->buf[i];
			i++;

			int trim_prev = str->buf[i] == '-';
			if (trim_prev)
				i++;
			if (trim_next) {
				_skip_while(str, &start, " \t\n");
				trim_next = 0;
			}

			if (!_skip_while(str, &i, " \t\n"))
				goto error;
			size_t s = i;
			if (!_skip_until(str, &i, " \t\n"))
				goto error;
			switch (c) {
			case '{':
				c = '}';
				string var = string_copy(str, s, i);
				if (var == NULL)
					goto error;
				t->vars[2*t->count + 1] = var;
				t->types[t->count] = CINJA_TYPE_SUBST;
				break;
			case '%':
				switch (i - s) {
				case 2:
					if (memcmp(&str->buf[s], "if", 2) == 0) {
						void *e = t->expr_ifs[t->count*2 + 1] = parse_if(str, &i);
						if (e == NULL)
							goto error;
						t->types[t->count] = CINJA_TYPE_IF;
					}
					break;
				case 3:
					if (memcmp(&str->buf[s], "end", 3) == 0) {
						t->types[t->count] = CINJA_TYPE_ENDIF;
					}
					break;
#if 0
				case 3:
					if (memcmp(&str->buf[i], "for", 3) == 0 {
						t->expr_fors[t->count] = parse_for(str, &i);
						if (t->expr_fors[t->count] == NULL)
							goto error;
						t->types[t->count] = CINJA_TYPE_FOR;
					}
					break;
#endif
				case 4:
					if (memcmp(&str->buf[s], "elif", 4) == 0) {
						void *e = t->expr_ifs[t->count*2 + 1] = parse_if(str, &i);
						if (e == NULL)
							goto error;
						t->types[t->count] = CINJA_TYPE_ELIF;
					} else if (memcmp(&str->buf[s], "else", 4) == 0) {
						t->types[t->count] = CINJA_TYPE_ELSE;
					} else {
						goto error;
					}
					break;
				default:
					goto error;
				}

				break;
			case '#':
				while (1) {
					if (str->buf[i] == '#' && str->buf[i+1] == '}')
						break;
					i++;
				}
				trim_next = str->buf[i-1] == '-';
				t->types[t->count] = CINJA_TYPE_COMMENT;
				goto comment;
			default:
				goto skip;
			}

			if (!_skip_while(str, &i, " \t\n"))
				goto error;
			trim_next = str->buf[i] == '-';
			if (trim_next)
				i++;
			if (str->buf[i] != c || str->buf[i + 1] != '}') {
				free(t->ptr[t->count*2 + 1]);
				goto error;
			}
		comment:
			if (trim_prev) {
				while (1) {
					char c = str->buf[end - 1];
					if (c != ' ' && c != '\t' && c != '\n')
						break;
					end--;
				}
			}
			t->text[2*t->count] = string_copy(str, start, end);
			i += 2;
			start = i;
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

	cinja_template t = cinja_create_from_string(str);
	free(str);
	return t;
}


void cinja_free(cinja_template temp)
{
	for (size_t i = 0; i < temp->count; i++) {
		free(temp->text[i*2]);
		switch (temp->types[i]) {
		case CINJA_TYPE_IF:
		case CINJA_TYPE_ELIF:
			free(temp->expr_ifs[i*2 + 1]->var);
			free(temp->expr_ifs[i*2 + 1]->val);
		case CINJA_TYPE_SUBST:
			free(temp->ptr[i*2 + 1]);
		case CINJA_TYPE_COMMENT:
		case CINJA_TYPE_ELSE:
		case CINJA_TYPE_ENDIF:
#if 0
		case CINJA_TYPE_ENDFOR:
#endif
			break;
		}
	}
	free(temp->text[temp->count*2]);
	free(temp->ptr);
	free(temp->types);
	free(temp);
}


string cinja_render(cinja_template temp, cinja_dict dict)
{
	size_t n = 1;
	string *strs = malloc(sizeof(*strs) * (2 * temp->count + 1));
	strs[0] = temp->text[0];
	char   skip_else[16];
	size_t skip_else_i = 0;
	size_t for_loop_index[16];
	void  *for_loop_state[16];
	size_t for_loop_i = 0;
	memset(for_loop_state, 0, sizeof(for_loop_state));

	for (size_t i = 0; i < temp->count; i++) {
		switch (temp->types[i]) {

		case CINJA_TYPE_SUBST: {
			cinja_dict_entry_t e = cinja_dict_get(dict, temp->vars[i*2 + 1]);
			strs[n] = e.value == NULL ? nullstr : e.value;
			n++;
			break;
		}

		case CINJA_TYPE_COMMENT:
			break;

		case CINJA_TYPE_ELIF:
			if (skip_else[skip_else_i]) {
				do {
					i++;
				} while (temp->types[i] != CINJA_TYPE_ENDIF);
			}
		case CINJA_TYPE_IF: {
			cinja_expr_if expr = temp->expr_ifs[i*2 + 1];
			cinja_dict_entry_t e = cinja_dict_get(dict, expr->var);
			int eq = expr->val == NULL && e.value == NULL;
			if (!eq && expr->val != NULL && e.value != NULL)
				eq = string_eq(e.value, expr->val);
			if (eq != (expr->op == CINJA_EXPR_OP_EQ)) {
				int f;
				do {
					i++;
					f = temp->types[i];
				} while (f != CINJA_TYPE_ENDIF && f != CINJA_TYPE_ELSE && f != CINJA_TYPE_ELIF);
			} else {
				skip_else[skip_else_i++] = 1;
			}
			break;
		}
		case CINJA_TYPE_ELSE:
			if (skip_else[skip_else_i]) {
				do {
					i++;
				} while (temp->types[i] != CINJA_TYPE_ENDIF);
			}
			break;
		case CINJA_TYPE_ENDIF:
			skip_else_i--;
			break;
#if 0
		case CINJA_TYPE_FOR:
			cinja_dict_entry_t entry = 
			break;
		case CINJA_TYPE_ENDFOR:
			if (for_loop_state[for_loop_i] == NULL) {
				for_loop_i--;
			} else {
				i = for_loop_index
			}
#endif
		default:
			return NULL;
			break; // TODO
		}
		strs[n] = temp->text[i*2 + 2];
		n++;
	}
	strs[n] = temp->text[temp->count*2];
	string render = string_concat(strs, n);
	free(strs);
	return render;
}
