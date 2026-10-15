#include <sys/types.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _citrus_module_rec *_citrus_module_t;

struct _citrus_ctype_ops {
    uint32_t co_abi_version;

    int (*co_init)(void **, void *, size_t, size_t);
    void (*co_uninit)(void *);
    unsigned (*co_get_mb_cur_max)(void *);

    void *co_mblen;
    void *co_mbrlen;
    void *co_mbrtowc;
    void *co_mbsinit;
    void *co_mbsrtowcs;
    void *co_mbstowcs;
    void *co_mbtowc;
    void *co_wcrtomb;
    void *co_wcsrtombs;
    void *co_wcstombs;
    void *co_wctomb;
    void *co_btowc;
    void *co_wctob;
    void *co_mbsnrtowcs;
    void *co_wcsnrtombs;
};

struct _citrus_module_rec {
    int mode;
};

static int unload_count;
static int init_count;
static int uninit_count;

static int
fake_init(void **closure, void *variable,
    size_t lenvar, size_t szpriv)
{
    (void)variable;
    (void)lenvar;
    (void)szpriv;

    init_count++;

    *closure = NULL;

    printf("FAULT: co_init called\n");

    return EINVAL;
}

static int
fake_success_init(void **closure, void *variable,
    size_t lenvar, size_t szpriv)
{
    (void)variable;
    (void)lenvar;
    (void)szpriv;

    init_count++;

    *closure = malloc(1);

    if (*closure == NULL)
        return ENOMEM;

    printf("FAULT: co_init succeeded\n");

    return 0;
}

static void
fake_uninit(void *closure)
{
    uninit_count++;

    printf("FAULT: co_uninit called\n");

    free(closure);
}

static unsigned
fake_mb_cur_max(void *closure)
{
    (void)closure;

    return 6;
}

static int
fake_callback(void)
{
    return 0;
}

static int
fake_getops(struct _citrus_ctype_ops *ops,
    size_t len, uint32_t version)
{
    if (len != sizeof(*ops) || version != 3)
        return EINVAL;

    memset(ops, 0, len);

    ops->co_abi_version = 3;
    ops->co_init = fake_init;
    ops->co_uninit = fake_uninit;
    ops->co_get_mb_cur_max = fake_mb_cur_max;

    /*
     * The remaining callbacks are not invoked by these tests.
     * Non-NULL addresses satisfy CTYPE validation.
     */
    ops->co_mblen = (void *)fake_callback;
    ops->co_mbrlen = (void *)fake_callback;
    ops->co_mbrtowc = (void *)fake_callback;
    ops->co_mbsinit = (void *)fake_callback;
    ops->co_mbsrtowcs = (void *)fake_callback;
    ops->co_mbstowcs = (void *)fake_callback;
    ops->co_mbtowc = (void *)fake_callback;
    ops->co_wcrtomb = (void *)fake_callback;
    ops->co_wcsrtombs = (void *)fake_callback;
    ops->co_wcstombs = (void *)fake_callback;
    ops->co_wctomb = (void *)fake_callback;
    ops->co_btowc = (void *)fake_callback;
    ops->co_wctob = (void *)fake_callback;
    ops->co_mbsnrtowcs = (void *)fake_callback;
    ops->co_wcsnrtombs = (void *)fake_callback;

    return 0;
}

static int
fake_badabi_getops(struct _citrus_ctype_ops *ops,
    size_t len, uint32_t version)
{
    int ret;

    ret = fake_getops(ops, len, version);

    if (ret != 0)
        return ret;

    ops->co_abi_version = 2;

    return 0;
}

static int
fake_success_getops(struct _citrus_ctype_ops *ops,
    size_t len, uint32_t version)
{
    int ret;

    ret = fake_getops(ops, len, version);

    if (ret != 0)
        return ret;

    ops->co_init = fake_success_init;

    return 0;
}

int
_citrus_load_module(_citrus_module_t *module, const char *name)
{
    struct _citrus_module_rec *handle;

    if (module == NULL || name == NULL)
        return EINVAL;

    *module = NULL;

    handle = calloc(1, sizeof(*handle));

    if (handle == NULL)
        return ENOMEM;

    if (strcmp(name, "FAULT_BADABI") == 0)
        handle->mode = 1;
    else if (strcmp(name, "FAULT_INIT") == 0)
        handle->mode = 2;
    else if (strcmp(name, "FAULT_SUCCESS") == 0)
        handle->mode = 3;
    else {
        free(handle);
        return EINVAL;
    }

    *module = handle;

    printf("FAULT: module '%s' loaded\n", name);

    return 0;
}

void *
_citrus_find_getops(_citrus_module_t module,
    const char *name, const char *interface)
{
    if (module == NULL || name == NULL || interface == NULL)
        return NULL;

    if (strcmp(interface, "ctype") != 0)
        return NULL;

    if (module->mode == 1 &&
        strcmp(name, "FAULT_BADABI") == 0)
        return (void *)fake_badabi_getops;

    if (module->mode == 2 &&
        strcmp(name, "FAULT_INIT") == 0)
        return (void *)fake_getops;

    if (module->mode == 3 &&
        strcmp(name, "FAULT_SUCCESS") == 0)
        return (void *)fake_success_getops;

    return NULL;
}

void
_citrus_unload_module(_citrus_module_t module)
{
    if (module == NULL)
        return;

    unload_count++;

    printf("FAULT: module unloaded\n");

    free(module);
}

void
fault_reset(void)
{
    unload_count = 0;
    init_count = 0;
    uninit_count = 0;
}

int
fault_unloads(void)
{
    return unload_count;
}

int
fault_inits(void)
{
    return init_count;
}

int
fault_uninits(void)
{
    return uninit_count;
}
