#include <stdio.h>
#include <string.h>

struct _citrus_stdenc;

extern int _citrus_stdenc_open(
    struct _citrus_stdenc **,
    const char *, const void *, size_t);

extern void fault_reset(void);
extern int fault_unloads(void);
extern int fault_uninits(void);
extern int fault_inits(void);

static int
test_failure(const char *name, int expected_inits)
{
    struct _citrus_stdenc *ce;
    int ret;

    printf("\n=== Testing %s ===\n", name);
    fflush(stdout);

    fault_reset();

    ce = NULL;

    ret = _citrus_stdenc_open(&ce, name, NULL, 0);

    printf("Return value: %d (%s)\n", ret, strerror(ret));
    printf("Result pointer: %p\n", (void *)ce);
    printf("Module unloads: %d\n", fault_unloads());
    printf("Initialization calls: %d\n", fault_inits());
    printf("Uninitialization calls: %d\n", fault_uninits());

    if (ret == 0 ||
        ce != NULL ||
        fault_unloads() != 1 ||
        fault_inits() != expected_inits ||
        fault_uninits() != 0) {
        printf("FAIL: unexpected failure cleanup\n");
        return 1;
    }

    printf("PASS: failure cleaned up correctly\n");

    return 0;
}

int
main(void)
{
    int failed;

    printf("Citrus STDENC controlled failure tests\n");
    fflush(stdout);

    failed = 0;

    failed |= test_failure("FAULT_BADABI", 0);
    failed |= test_failure("FAULT_INIT", 1);

    printf("\nControlled failure tests: %s\n",
        failed ? "FAILED" : "SUCCESS");

    return failed ? 1 : 0;
}
