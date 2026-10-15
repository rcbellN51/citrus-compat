#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>

typedef int (*esdb_open_func)(void *, const char *);

int
_citrus_esdb_open(void *result, const char *encoding)
{
    esdb_open_func original;
    void *symbol;
    int error;

    fprintf(stderr, "ESDB: opening '%s'\n",
        encoding != NULL ? encoding : "(null)");

    dlerror();
    symbol = dlsym(RTLD_NEXT, "_citrus_esdb_open");

    if (symbol == NULL) {
        fprintf(stderr, "ESDB: RTLD_NEXT failed: %s\n",
            dlerror());
        return EINVAL;
    }

    original = (esdb_open_func)symbol;
    error = original(result, encoding);

    fprintf(stderr, "ESDB: '%s' returned %d\n",
        encoding != NULL ? encoding : "(null)", error);

    return error;
}
