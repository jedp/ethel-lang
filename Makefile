
MEMOBJS  = src/map.o

COMPOBJS = src/lexer.o \
	   src/parser.o

REPLOBJS = src/basic.o

TESTOBJS = test/unity/unity.o \
	   test/test_map.o \
	   test/test_lexer.o \
	   test/test_parser.o \
	   test/test.o

CFLAGS = -std=gnu11 -Wall -m32 -g -O0 -I inc
TESTFLAGS = -I test
LDFLAGS = -lm -lreadline -ldl

all: basic

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

basic: $(MEMOBJS) $(REPLOBJS) $(COMPOBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(MEMOBJS) $(COMPOBJS) $(TESTOBJS)
	$(CC) $(CFLAGS) $(TESTFLAGS) -o $@/test $^ $(LDFLAGS)
	./test/test

wc:
	find . -name "*.[ch]" | xargs wc -l | sort -n

.PHONY: clean
clean:
	rm -f $(MEMOBJS) $(COMPOBJS) $(REPLOBJS) $(TESTOBJS)
	rm -f basic test/test

