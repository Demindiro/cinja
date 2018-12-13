#include <stdio.h>
#include <stdlib.h>
#include "../lib/string/include/string.h"
#include "../include/cinja.h"


static const char **files;
static size_t       files_count;
static size_t       files_index = -1;


static const char *get_file()
{
	files_index = (files_index + 1) % files_count;
	return files[files_index];
}


static int render(const char *foo, const char *bar, const char *html)
{
	cinja_dict d = cinja_dict_create();
	cinja_dict_set(d, string_create("FOO" ), foo  == NULL ? NULL : string_create(foo ));
	cinja_dict_set(d, string_create("BAR" ), bar  == NULL ? NULL : string_create(bar ));
	cinja_dict_set(d, string_create("HTML"), html == NULL ? NULL : string_create(html));

	cinja_template temp = cinja_create_from_file(get_file());
	if (temp == NULL) {
		fprintf(stderr, "Creation failed\n");
		goto error;
	}
	string render = cinja_render(temp, d);
	if (render == NULL) {
		fprintf(stderr, "Render failed\n");
		goto error;
	}

	int r = 0;
	goto success;
error:
	r = -1;
success:
	puts(render->buf);
	free(render);
	cinja_free(temp);
	cinja_dict_free(d);
	return r;
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

	puts("0: FOO, BAR, HTML defined (BAR == 'baz')");
	if (render("Lorem ispum dolor", "baz", "<body><h1>Hello world!</h1></body>") < 0)
		return -1;
	puts("1: BAR, HTML defined, FOO undefined (BAR == 'baz')");
	if (render(NULL, "baz", "<h2>The test is going well</h2>") < 0)
		return -1;
	puts("2: BAR, HTML defined, FOO undefined (BAR == 'barium')");
	if (render(NULL, "barium", "<h2>The test is going really well</h2>") < 0)
		return -1;
	puts("3: FOO, HTML defined, BAR undefined");
	if (render("foos roh dah", NULL, "<h2>The test is going well</h2>") < 0)
		return -1;
	puts("4: FOO, BAR, HTML undefined");
	if (render(NULL, NULL, NULL) < 0)
		return -1;

	return 0;
}
