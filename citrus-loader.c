#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

typedef struct _citrus_module_rec *_citrus_module_t;

#define CITRUS_MODULE_DIR "/usr/lib/i18n"
#define CITRUS_NAME_MAX 128

int
_citrus_load_module(_citrus_module_t *rhandle, const char *name)
{
    char path[256];
    void *handle;
    size_t i;
    int length;

    if (rhandle == NULL || name == NULL)
        return EINVAL;

    *rhandle = NULL;

    if (name[0] == '\0')
        return EINVAL;

    for (i = 0; name[i] != '\0'; i++) {
        if (i >= CITRUS_NAME_MAX - 1)
            return EINVAL;

        if (!((name[i] >= 'A' && name[i] <= 'Z') ||
              (name[i] >= 'a' && name[i] <= 'z') ||
              (name[i] >= '0' && name[i] <= '9') ||
              name[i] == '_'))
            return EINVAL;
    }

    length = snprintf(path, sizeof(path),
        "%s/lib%s.so", CITRUS_MODULE_DIR, name);

    if (length < 0 || (size_t)length >= sizeof(path))
        return EINVAL;

    fprintf(stderr, "CITRUS: loading %s\n", path);

    handle = dlopen(path, RTLD_LAZY);

    if (handle == NULL) {
        fprintf(stderr, "CITRUS: dlopen failed: %s\n", dlerror());
        return EINVAL;
    }

    fprintf(stderr, "CITRUS: module loaded successfully\n");

    *rhandle = (_citrus_module_t)handle;

    return 0;
}

void *
_citrus_find_getops(_citrus_module_t handle,
    const char *modname, const char *ifname)
{
    char symbol[256];
    void *getops;
    int length;

    if (handle == NULL || modname == NULL || ifname == NULL)
        return NULL;

    length = snprintf(symbol, sizeof(symbol),
        "_citrus_%s_%s_getops", modname, ifname);

    if (length < 0 || (size_t)length >= sizeof(symbol))
        return NULL;

    fprintf(stderr, "CITRUS: resolving %s\n", symbol);

    dlerror();
    getops = dlsym((void *)handle, symbol);

    if (getops == NULL) {
        fprintf(stderr, "CITRUS: dlsym failed: %s\n", dlerror());
    } else {
        fprintf(stderr, "CITRUS: symbol resolved successfully\n");
    }

    return getops;
}

void
_citrus_unload_module(_citrus_module_t handle)
{
    if (handle != NULL) {
        fprintf(stderr, "CITRUS: unloading module\n");
        dlclose((void *)handle);
    }
}

