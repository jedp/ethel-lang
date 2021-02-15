
COMPOBJS = src/lexer.o \
					 src/ast.o \
					 src/parser.o \
					 src/eval.o

REPLOBJS = src/repl.o

TESTOBJS = test/unity/unity.o \
					 test/test_lexer.o \
					 test/test_parser.o \
					 test/test_eval.o \
					 test/test.o

CFLAGS = -std=gnu11 -Wall -g -O0 -I inc
TESTFLAGS = -I test
LDFLAGS = -lm -lreadline -ldl

all: repl

debug: CFLAGS += -DDEBUG
debug: all

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

repl: $(REPLOBJS) $(COMPOBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(COMPOBJS) $(TESTOBJS)
	$(CC) $(CFLAGS) $(TESTFLAGS) -o $@/test $^ $(LDFLAGS)
	./test/test

wc:
	find . -name "*.[ch]" | xargs wc -l | sort -n

.PHONY: all clean debug
clean:
	rm -f $(COMPOBJS) $(REPLOBJS) $(TESTOBJS)
	rm -f repl test/test

