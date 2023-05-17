.PHONY: all
all: task stdinExample threadpool

threadpool: main.c
	gcc main.c -L. -l Codec threadpool

task:	codec.h basic_main.c
	gcc basic_main.c -L. -l Codec -o encoder

stdinExample:	stdin_main.c
		gcc stdin_main.c -L. -l Codec -o tester

.PHONY: clean
clean:
	-rm encoder tester threadpool libCodec.so 2>/dev/null
