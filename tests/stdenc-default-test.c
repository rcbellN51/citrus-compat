#include <stdio.h>
#include <dlfcn.h>

struct _citrus_stdenc;

extern struct _citrus_stdenc _citrus_stdenc_default;

typedef int (*stdenc_open_t)(
    struct _citrus_stdenc **,
    const char *,
    const void *,
    size_t);

int
main(void)
{
    struct _citrus_stdenc *ce = NULL;
    stdenc_open_t original;
    int ret;

    original = (stdenc_open_t)dlsym(
        RTLD_NEXT, "_citrus_stdenc_open");

    if (original == NULL) {
        fprintf(stderr, "Unable to locate libc stdenc_open\n");
        return 1;
    }

    ret = original(&ce, "NONE", NULL, 0);

    printf("Return value: %d\n", ret);
    printf("Returned object: %p\n", (void *)ce);
    printf("Default object:  %p\n",
        (void *)&_citrus_stdenc_default);
    printf("Same object:     %s\n",
        ce == &_citrus_stdenc_default ? "yes" : "no");

    return ret != 0 || ce != &_citrus_stdenc_default;
}
