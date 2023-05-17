.PHONY: all
all: task stdinExample coder
	export LD_LIBRARY_PATH=.


coder: main.c codec.h
	gcc main.c -L. -l Codec -o coder -pthread

task:	codec.h basic_main.c
	gcc basic_main.c -L. -l Codec -o encoder

stdinExample:	stdin_main.c codec.h
		gcc stdin_main.c -L. -l Codec -o tester

.PHONY: clean
clean:
	-rm encoder tester coder 2>/dev/null
