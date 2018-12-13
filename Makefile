ifndef OPT_LEVEL
	OPT_LEVEL = 2
endif

ifndef ARGS
	ARGS = test.html
endif

ifdef DB
	OPT_LEVEL = 0
	C_FLAGS = -g
endif

run_test:
	[ -e build/ ] || mkdir build/
	gcc -O$(OPT_LEVEL) $(C_FLAGS) src/*.c test/*.c lib/*/src/*.c -o build/test
	cd test; $(DB) $(DB_ARGS) ../build/test $(ARGS)
