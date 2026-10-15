#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>

typedef int (*stdenc_open_func)(void *, const char *,
    const void *, size_t);

int
_citrus_stdenc_open(void *result, const char *encoding,
    const void *variable, size_t variable_length)
{
    stdenc_open_func original;
    void *symbol;
    int error;

    fprintf(stderr, "STDENC: opening '%s'\n",
        encoding != NULL ? encoding : "(null)");

    dlerror();
    symbol = dlsym(RTLD_NEXT, "_citrus_stdenc_open");

    if (symbol == NULL) {
        fprintf(stderr, "STDENC: RTLD_NEXT failed: %s\n",
            dlerror());
        return EINVAL;
    }

    original = (stdenc_open_func)symbol;

    error = original(result, encoding, variable, variable_length);

    fprintf(stderr, "STDENC: '%s' returned %d\n",
        encoding != NULL ? encoding : "(null)", error);

    return error;
}
