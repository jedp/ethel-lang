
COMPOBJS = src/lexer.o \
					 src/ast.o \
					 src/parser.o \
					 src/obj.o \
					 src/eval.o \
					 src/env.o

REPLOBJS = src/repl.o

TESTOBJS = test/unity/unity.o \
					 test/test_lexer.o \
					 test/test_parser.o \
					 test/test_eval.o \
					 test/test_env.o \
					 test/test.o

CFLAGS = -std=gnu11 -Wall -g3 -Os -I inc
EXTRA_CFLAGS = -Wshadow -Wdouble-promotion -Wconversion -Wformat -Wno-undef -fno-common
TESTFLAGS = -I test
LDFLAGS = -lm -lreadline -ldl

all: test repl

debug: CFLAGS += -DDEBUG
debug: all

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

repl: $(REPLOBJS) $(COMPOBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(COMPOBJS) $(TESTOBJS)
	$(CC) $(CFLAGS) $(TESTFLAGS) -o $@/test $^ $(LDFLAGS)
	./test/test

wc:
	find . -name "*.[ch]" | xargs wc -l | sort -n

.PHONY: all clean test debug
clean:
	rm -f $(COMPOBJS) $(REPLOBJS) $(TESTOBJS)
	rm -f repl test/test

