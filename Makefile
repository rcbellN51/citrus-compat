# Citrus compatibility layer for MINIX 3
#
# Builds a project-local static library and regression tests.
# Does not install or modify system libraries.

CC?=            cc
AR?=            ar

CFLAGS+=        -Wall -Wextra

LIB=            libcitrus-compat.a

SRCS=           citrus-loader.c \
                citrus-stdenc.c \
                citrus-ctype.c

OBJS=           ${SRCS:.c=.o}

# Rebuild these objects when the debug header changes.
citrus-loader.o citrus-ctype.o citrus-stdenc.o: citrus-debug.h

TESTS=          tests/integration-test \
                tests/integration-whole \
                tests/error-test \
                tests/stdenc-error-test \
                tests/stdenc-fault-test \
                tests/ctype-fault-test

all: ${LIB}

${LIB}: ${OBJS}
	${AR} rcs ${.TARGET} ${OBJS}

tests/integration-test: tests/integration-test.c ${SRCS}
	${CC} ${CFLAGS} -o ${.TARGET} tests/integration-test.c ${SRCS}

tests/integration-whole: tests/integration-test.c ${LIB}
	${CC} ${CFLAGS} -o ${.TARGET} tests/integration-test.c \
	    -Wl,--whole-archive ${LIB} -Wl,--no-whole-archive

tests/error-test: tests/error-test.c ${SRCS}
	${CC} ${CFLAGS} -o ${.TARGET} tests/error-test.c ${SRCS}

tests/stdenc-error-test: tests/stdenc-error-test.c \
    citrus-loader.c citrus-stdenc.c
	${CC} ${CFLAGS} -o ${.TARGET} tests/stdenc-error-test.c \
	    citrus-loader.c citrus-stdenc.c

tests/stdenc-fault-test: tests/stdenc-fault-test.c \
    tests/stdenc-fault-loader.c citrus-stdenc.c
	${CC} ${CFLAGS} -o ${.TARGET} tests/stdenc-fault-test.c \
	    tests/stdenc-fault-loader.c citrus-stdenc.c

tests/ctype-fault-test: tests/ctype-fault-test.c \
    tests/ctype-fault-loader.c citrus-ctype.c
	${CC} ${CFLAGS} -o ${.TARGET} tests/ctype-fault-test.c \
	    tests/ctype-fault-loader.c citrus-ctype.c

examples/utf8-demo: examples/utf8-demo.c ${LIB}
	${CC} ${CFLAGS} -o ${.TARGET} examples/utf8-demo.c \
	    -Wl,--whole-archive ${LIB} -Wl,--no-whole-archive

examples: examples/utf8-demo

tests: ${TESTS}

check: tests
	ICONV_MAX_REUSE=0 ./tests/integration-test
	ICONV_MAX_REUSE=0 ./tests/integration-whole
	ICONV_MAX_REUSE=0 ./tests/error-test
	./tests/stdenc-error-test
	./tests/stdenc-fault-test
	./tests/ctype-fault-test

clean:
	rm -f ${OBJS} ${LIB} ${TESTS} examples/utf8-demo

.PHONY: all tests check examples clean
