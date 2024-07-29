cmake_minimum_required(VERSION "3.13")
project("ethel-lang" C)

set(CMAKE_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(CMAKE_INC_DIR ${CMAKE_SOURCE_DIR}/inc)
set(CMAKE_TESTS_DIR ${CMAKE_SOURCE_DIR}/tests)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -g3 -Os -I ${CMAKE_INC_DIR} -I ${CMAKE_SRC_DIR}/common")
set(CMAKE_C_FLAGS_TEST "${CMAKE_C_FLAGS_TEST} = -std=c11 -g3 -I ${CMAKE_INC_DIR}")
set(EXTRA_CFLAGS
        -Wall
        -Wformat
        -Wshadow
        -Wno-undef
        -Wcast-qual
        -Wcast-align
        -Wconversion
        -Wdouble-promotion
        -Wpointer-arith
        -Wwrite-strings
        -Wint-to-pointer-cast
        --pedantic-errors
        -fverbose-asm
        -fno-common
        -fno-omit-frame-pointer
        -fsanitize=address
)
set(TESTFLAGS -I test -fno-omit-frame-pointer -fsanitize=address)
set(LDFLAGS -lm -lreadline -ldl)

set(COMMON_OBJS
        ${CMAKE_SRC_DIR}/common/hash.c
        ${CMAKE_SRC_DIR}/common/map.c
        ${CMAKE_SRC_DIR}/common/math.c
        ${CMAKE_SRC_DIR}/common/ptr.c
        ${CMAKE_SRC_DIR}/common/rand.c
        ${CMAKE_SRC_DIR}/common/val.c
)

set(MEM_OBJS
        ${CMAKE_SRC_DIR}/mem/mem.c
)

set(OBJ_OBJS
        ${COMMON_OBJS}
        ${MEM_OBJS}
        ${CMAKE_SRC_DIR}/obj/obj.c
        ${CMAKE_SRC_DIR}/obj/obj_str.c
        ${CMAKE_SRC_DIR}/obj/obj_arr.c
)

set(DIS_OBJS
        ${CMAKE_SRC_DIR}/dis/dis.c
)

set(COMP_OBJS
        ${OBJ_OBJS}
        ${DIS_OBJS}
        ${CMAKE_SRC_DIR}/comp/cg.c
        ${CMAKE_SRC_DIR}/comp/comp.c
        ${CMAKE_SRC_DIR}/comp/lex.c
)

set(EC_OBJS
        ${CMAKE_SRC_DIR}/comp/ec.c
)

set(ER_OBJS
        ${CMAKE_SRC_DIR}/run/er.c
)

set(VM_OBJS
        ${CMAKE_SRC_DIR}/vm/ert.c
        ${CMAKE_SRC_DIR}/vm/load.c
        ${CMAKE_SRC_DIR}/vm/vm.c
)

set(TEST_OBJS
        ${CMAKE_TESTS_DIR}/unity/unity.c
        ${CMAKE_TESTS_DIR}/test_ptr.c
        ${CMAKE_TESTS_DIR}/test_cg.c
        ${CMAKE_TESTS_DIR}/test_comp.c
        ${CMAKE_TESTS_DIR}/test_dis.c
        ${CMAKE_TESTS_DIR}/test_lex.c
        ${CMAKE_TESTS_DIR}/test_map.c
        ${CMAKE_TESTS_DIR}/test_vm.c
        ${CMAKE_TESTS_DIR}/test.c
)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Create output directories
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

# Ethel Compile
add_executable(ec
        ${COMP_OBJS}
        ${COMMON_OBJS}
        ${EC_OBJS}
)

# Ethel Run
add_executable(er
        ${COMP_OBJS}
        ${COMMON_OBJS}
        ${ER_OBJS}
)

# Tests
add_executable(test
        ${COMP_OBJS}
        ${COMMON_OBJS}
        ${VM_OBJS}
        ${TEST_OBJS}
)

# Test executable
add_custom_target(tests
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test
        DEPENDS test
)

# Word count
add_custom_target(wc
        COMMAND find ${CMAKE_SOURCE_DIR} -name '*.[ch]'
        | grep -v unity
        | grep -v CMake
        | xargs wc -l
        | sort -n
)

# Make all runs tests
add_custom_target(all_and_test ALL
        DEPENDS tests
)
