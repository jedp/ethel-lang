
COMPOBJS = src/ptr.o \
					 src/heap.o \
					 src/mem.o \
					 src/obj.o \
					 src/math.o \
					 src/range.o \
					 src/arr.o \
					 src/str.o \
					 src/float.o \
					 src/byte.o \
					 src/int.o \
					 src/list.o \
					 src/dict.o \
					 src/bool.o \
					 src/fn.o \
					 src/type.o \
					 src/rand.o \
					 src/lexer.o \
					 src/parser.o \
					 src/env.o \
					 src/ast.o \
					 src/eval.o

REPLOBJS = src/repl.o

RUNOBJS = src/run.o

TESTOBJS = test/unity/unity.o \
					 test/util.o \
					 test/test_hash.o \
					 test/test_str.o \
					 test/test_list.o \
					 test/test_dict.o \
					 test/test_range.o \
					 test/test_ptr.o \
					 test/test_heap.o \
					 test/test_trace.o \
					 test/test_bytearray.o \
					 test/test_lexer.o \
					 test/test_parser.o \
					 test/test_eval.o \
					 test/test_env.o \
					 test/test_rand.o \
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
 -fverbose-asm \
 -fno-common \
 -fno-omit-frame-pointer \
 -fsanitize=address
TESTFLAGS = -I test -fno-omit-frame-pointer -fsanitize=address
LDFLAGS = -lm -lreadline -ldl

all: test repl run

debug: CFLAGS += -DDEBUG
debug: repl

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

repl: $(REPLOBJS) $(COMPOBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(RUNOBJS) $(COMPOBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(COMPOBJS) $(TESTOBJS)
	$(CC) $(CFLAGS) $(TESTFLAGS) -o $@/test $^ $(LDFLAGS)
	./test/test

wc:
	find . -name "*.[ch]" | xargs wc -l | sort -n

.PHONY: all clean test debug
clean:
	rm -f $(COMPOBJS) $(REPLOBJS) $(RUNOBJS) $(TESTOBJS)
	rm -f repl test/test

