#include <errno.h>
#include <stdio.h>
#include <string.h>

struct _citrus_ctype;

extern int _citrus_ctype_open(
    struct _citrus_ctype **,
    const char *, void *, size_t, size_t);

extern void _citrus_ctype_close(struct _citrus_ctype *);

extern void fault_reset(void);
extern int fault_unloads(void);
extern int fault_inits(void);
extern int fault_uninits(void);

static int
test_failure(const char *name, int expected_inits)
{
    struct _citrus_ctype *cc;
    int ret;

    printf("\n=== Testing %s ===\n", name);
    fflush(stdout);

    fault_reset();

    cc = NULL;

    ret = _citrus_ctype_open(&cc, name, NULL, 0, 112);

    printf("Return value: %d (%s)\n", ret, strerror(ret));
    printf("Result pointer: %p\n", (void *)cc);
    printf("Module unloads: %d\n", fault_unloads());
    printf("Initialization calls: %d\n", fault_inits());
    printf("Uninitialization calls: %d\n", fault_uninits());

    if (ret == 0 ||
        cc != NULL ||
        fault_unloads() != 1 ||
        fault_inits() != expected_inits ||
        fault_uninits() != 0) {
        printf("FAIL: unexpected failure cleanup\n");
        return 1;
    }

    printf("PASS: failure cleaned up correctly\n");

    return 0;
}

static int
test_success(void)
{
    struct _citrus_ctype *cc;
    int ret;

    printf("\n=== Testing FAULT_SUCCESS ===\n");
    fflush(stdout);

    fault_reset();

    cc = NULL;

    ret = _citrus_ctype_open(
        &cc, "FAULT_SUCCESS", NULL, 0, 112);

    if (ret != 0 || cc == NULL) {
        printf("FAIL: initialization failed: %d\n", ret);
        return 1;
    }

    printf("PASS: initialization succeeded\n");

    if (fault_inits() != 1 ||
        fault_uninits() != 0 ||
        fault_unloads() != 0) {
        printf("FAIL: unexpected state before close\n");
        _citrus_ctype_close(cc);
        return 1;
    }

    _citrus_ctype_close(cc);

    printf("Module unloads: %d\n", fault_unloads());
    printf("Initialization calls: %d\n", fault_inits());
    printf("Uninitialization calls: %d\n", fault_uninits());

    if (fault_unloads() != 1 ||
        fault_inits() != 1 ||
        fault_uninits() != 1) {
        printf("FAIL: unexpected successful cleanup\n");
        return 1;
    }

    printf("PASS: successful initialization cleaned up correctly\n");

    return 0;
}

int
main(void)
{
    int failed;

    printf("Citrus CTYPE controlled lifecycle tests\n");
    fflush(stdout);

    failed = 0;

    failed |= test_failure("FAULT_BADABI", 0);
    failed |= test_failure("FAULT_INIT", 1);
    failed |= test_success();

    printf("\nCTYPE lifecycle tests: %s\n",
        failed ? "FAILED" : "SUCCESS");

    return failed ? 1 : 0;
}
