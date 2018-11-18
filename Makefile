ifndef OPT_LEVEL
	OPT_LEVEL = 2
endif

ifndef ARGS
	ARGS = test.html
endif

run_test:
	[ -e build/ ] || mkdir build/
	gcc -O$(OPT_LEVEL) $(C_FLAGS) src/*.c test/*.c lib/*/src/*.c -o build/test
	cd test; $(DB) $(DB_ARGS) ../build/test $(ARGS)
