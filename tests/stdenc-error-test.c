#include <errno.h>
#include <stdio.h>
#include <string.h>

struct _citrus_stdenc;

extern int _citrus_stdenc_open(
    struct _citrus_stdenc **,
    const char *, const void *, size_t);

static int
test_encoding(const char *name)
{
    struct _citrus_stdenc *ce;
    int ret;

    printf("\n=== Testing '%s' ===\n", name);
    fflush(stdout);

    ce = NULL;

    ret = _citrus_stdenc_open(&ce, name, NULL, 0);

    printf("Return value: %d (%s)\n",
        ret, strerror(ret));
    printf("Result pointer: %p\n", (void *)ce);

    if (ret == 0 || ce != NULL) {
        printf("FAIL: invalid STDENC interface accepted\n");
        return 1;
    }

    printf("PASS: invalid STDENC interface rejected safely\n");

    return 0;
}

int
main(void)
{
    int failed;

    printf("Citrus STDENC failure-path tests\n");
    fflush(stdout);

    failed = 0;

    failed |= test_encoding("CITRUS-NONEXISTENT");
    failed |= test_encoding("mapper_none");

    printf("\nSTDENC error tests: %s\n",
        failed ? "FAILED" : "SUCCESS");

    return failed ? 1 : 0;
}
