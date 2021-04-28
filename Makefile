
COMPOBJS = src/mem.o \
					 src/obj.o \
					 src/math.o \
					 src/num.o \
					 src/range.o \
					 src/arr.o \
					 src/str.o \
					 src/float.o \
					 src/int.o \
					 src/list.o \
					 src/bool.o \
					 src/type.o \
					 src/lexer.o \
					 src/parser.o \
					 src/env.o \
					 src/ast.o \
					 src/eval.o

REPLOBJS = src/repl.o

TESTOBJS = test/unity/unity.o \
					 test/test_str.o \
					 test/test_list.o \
					 test/test_range.o \
					 test/test_mem.o \
					 test/test_bytearray.o \
					 test/test_lexer.o \
					 test/test_parser.o \
					 test/test_eval.o \
					 test/test_hash.o \
					 test/test_env.o \
					 test/test_examples.o \
					 test/test.o

CFLAGS = -std=gnu11 -g3 -Os -I inc
EXTRA_CFLAGS = \
 -Wall \
 -Wformat \
 -Wshadow \
 -Wno-undef \
 -Wcast-qual \
 -Wcast-align \
 -Wconversion \
 -Wdouble-promotion \
 -Wpointer-arith \
 -Wwrite-strings \
 -Wint-to-pointer-cast \
 --pedantic-errors \
 -fno-common \
 -fno-omit-frame-pointer \
 -fsanitize=address
TESTFLAGS = -I test -fno-omit-frame-pointer -fsanitize=address
LDFLAGS = -lm -lreadline -ldl

all: test repl

debug: CFLAGS += -DDEBUG
debug: repl

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

