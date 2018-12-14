#ifndef CINJA_TYPES_H__
#define CINJA_TYPES_H__

enum cinja_type {
	UNDEFINED,
	STRING,
	DICT,
	TEMPLATE,
	LIST,
};

#define GET_TYPE(value) \
	_Generic((value), default       : UNDEFINED, \
	                  string        : STRING, \
	                  cinja_dict    : DICT, \
	                  cinja_template: TEMPLATE, \
	                  cinja_list    : LIST)


#endif
