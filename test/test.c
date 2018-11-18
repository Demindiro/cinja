#include <stdio.h>
#include <stdlib.h>
#include "../lib/string/include/string.h"
#include "../include/cinja.h"


static const char **files;
static size_t       files_count;
static size_t       files_index = -1;


const char *get_file()
{
	files_index = (files_index + 1) % files_count;
	return files[files_index];
}


int main(int argc, const char **argv)
{
	if (argc < 2) {
		const char *cmd = argc == 1 ? argv[0] : "test";
		fprintf(stderr, "Usage: %s <files>\n", cmd);
		return 1;
	}
	files = argv + 1;
	files_count = argc - 1;

	for (size_t i = 0; i < 10; i++) {
		printf(" --- %lu --- \n", i);
		cinja_dict d = cinja_dict_create();
		cinja_dict_set(d, string_create("lorem"), string_create("Lorem ispum dolor"));
		cinja_dict_set(d, string_create("foo"  ), string_create("Foo bar baz"));
		cinja_dict_set(d, string_create("HTML" ), string_create("<body><h1>Hello world!</h1></body>"));
		
		cinja_template temp = cinja_create_from_file(get_file());
		string render = cinja_render(temp, d);
		puts(render->buf);
	}

	return 0;
}
