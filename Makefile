
COMMONOBJS = \
					 src/common/math.o \
					 src/common/rand.o \
					 src/common/map.o \

COMPOBJS = \
					 src/comp/cmem.o \
					 src/comp/comp.o \
					 src/comp/lex.o \
					 src/comp/comp.o \
					 src/comp/cg.o

ECOBJS = \
					 src/comp/ec.o

VMOBJS = \
					 src/vm/dis.o \
					 src/vm/vm.o

REPLOBJS = src/repl/repl.o

TESTOBJS = test/unity/unity.o \
					 test/test_map.o \
					 test/test_lex.o \
					 test/test_rand.o \
					 test/test_cg.o \
					 test/test_dis.o \
					 test/test_vm.o \
					 test/test.o

CFLAGS = -std=gnu11 -g3 -Os -I inc
CFLAGS_TEST = -std=gnu11 -g3 -I inc
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

all: test repl ec

debug: CFLAGS += -DDEBUG
debug: repl

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

ec: $(COMPOBJS) $(COMMONOBJS) $(ECOBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^ $(LDFLAGS)

repl: $(REPLOBJS) $(VMOBJS) $(COMPOBJS) $(COMMONOBJS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(COMMONOBJS) $(COMPOBJS) $(VMOBJS) $(TESTOBJS)
	$(CC) $(CFLAGS_TEST) $(TESTFLAGS) -o $@/test $^ $(LDFLAGS)
	./test/test

wc:
	find . -name "*.[ch]" | xargs wc -l | sort -n

.PHONY: clean all test ec repl debug
clean:
	rm -f $(COMMONOBJS) $(COMPOBJS) $(VMOBJS) $(REPLOBJS) $(RUNOBJS) $(TESTOBJS)
	rm -f repl test/test

