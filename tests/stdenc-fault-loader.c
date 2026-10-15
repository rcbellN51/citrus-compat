#include <sys/types.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _citrus_module_rec *_citrus_module_t;

struct _citrus_stdenc;
struct _citrus_stdenc_traits;

struct _citrus_stdenc_ops {
    uint32_t eo_abi_version;

    int (*eo_init)(struct _citrus_stdenc *,
        const void *, size_t, struct _citrus_stdenc_traits *);

    void (*eo_uninit)(struct _citrus_stdenc *);

    int (*eo_init_state)(struct _citrus_stdenc *, void *);

    int (*eo_mbtocs)(void);
    int (*eo_cstomb)(void);
    int (*eo_mbtowc)(void);
    int (*eo_wctomb)(void);
    int (*eo_put_state_reset)(void);
    int (*eo_get_state_desc)(void);
};

struct _citrus_stdenc_traits {
    size_t et_state_size;
    size_t et_mb_cur_max;
};

struct _citrus_module_rec {
    int mode;
};

static int unload_count;
static int uninit_count;
static int init_count;

static int
fake_init(struct _citrus_stdenc *ce, const void *variable,
    size_t lenvar, struct _citrus_stdenc_traits *traits)
{
    (void)ce;
    (void)variable;
    (void)lenvar;
    (void)traits;

    init_count++;

    printf("FAULT: eo_init called; returning EINVAL\n");

    return EINVAL;
}

static void
fake_uninit(struct _citrus_stdenc *ce)
{
    (void)ce;

    uninit_count++;

    printf("FAULT: eo_uninit called\n");
}

static int
fake_getops(struct _citrus_stdenc_ops *ops,
    size_t len, uint32_t version)
{
    if (len != sizeof(*ops) || version != 2)
        return EINVAL;

    memset(ops, 0, len);

    ops->eo_abi_version = 2;
    ops->eo_init = fake_init;
    ops->eo_uninit = fake_uninit;

    /*
     * These callbacks are not invoked by the test.
     * Non-NULL addresses satisfy STDENC validation.
     */
    ops->eo_init_state = (void *)fake_init;
    ops->eo_mbtocs = (void *)fake_init;
    ops->eo_cstomb = (void *)fake_init;
    ops->eo_mbtowc = (void *)fake_init;
    ops->eo_wctomb = (void *)fake_init;
    ops->eo_get_state_desc = (void *)fake_init;

    return 0;
}

static int
fake_badabi_getops(struct _citrus_stdenc_ops *ops,
    size_t len, uint32_t version)
{
    int ret;

    ret = fake_getops(ops, len, version);

    if (ret != 0)
        return ret;

    ops->eo_abi_version = 1;

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

    if (strcmp(interface, "stdenc") != 0)
        return NULL;

    if (module->mode == 1 &&
        strcmp(name, "FAULT_BADABI") == 0)
        return (void *)fake_badabi_getops;

    if (module->mode == 2 &&
        strcmp(name, "FAULT_INIT") == 0)
        return (void *)fake_getops;

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
    uninit_count = 0;
    init_count = 0;
}

int
fault_unloads(void)
{
    return unload_count;
}

int
fault_uninits(void)
{
    return uninit_count;
}

int
fault_inits(void)
{
    return init_count;
}
