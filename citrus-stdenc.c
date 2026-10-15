#include <sys/types.h>
#include <stdint.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _citrus_module_rec *_citrus_module_t;

struct _citrus_stdenc;
struct _citrus_stdenc_traits;

extern struct _citrus_stdenc _citrus_stdenc_default;

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

struct _citrus_stdenc {
    struct _citrus_stdenc_ops *ce_ops;
    void *ce_closure;
    _citrus_module_t ce_module;
    struct _citrus_stdenc_traits *ce_traits;
};

typedef int (*stdenc_getops_t)(
    struct _citrus_stdenc_ops *, size_t, uint32_t);

typedef int (*stdenc_open_t)(
    struct _citrus_stdenc **, const char *, const void *, size_t);

extern int _citrus_load_module(_citrus_module_t *, const char *);

extern void *_citrus_find_getops(
    _citrus_module_t, const char *, const char *);

extern void _citrus_unload_module(_citrus_module_t);

static stdenc_open_t
original_stdenc_open(void)
{
    return (stdenc_open_t)dlsym(RTLD_NEXT, "_citrus_stdenc_open");
}

static void
stdenc_destroy(struct _citrus_stdenc *ce, int initialized)
{
    if (ce == NULL || ce == &_citrus_stdenc_default)
        return;

    fprintf(stderr, "STDENC: closing dynamic encoding\n");

    if (ce->ce_ops != NULL) {
        if (initialized && ce->ce_ops->eo_uninit != NULL)
            ce->ce_ops->eo_uninit(ce);

        free(ce->ce_ops);
    }

    free(ce->ce_traits);

    if (ce->ce_module != NULL)
        _citrus_unload_module(ce->ce_module);

    free(ce);
}

void
_citrus_stdenc_close(struct _citrus_stdenc *ce)
{
    fprintf(stderr, "STDENC: close called (%p)\n", (void *)ce);

    stdenc_destroy(ce, 1);
}

int
_citrus_stdenc_open(struct _citrus_stdenc **rce,
    const char *encname, const void *variable, size_t lenvar)
{
    struct _citrus_stdenc *ce;
    stdenc_getops_t getops;
    stdenc_open_t original;
    int ret;

    if (rce == NULL || encname == NULL)
        return EINVAL;

    if (lenvar != 0 && variable == NULL)
        return EINVAL;

    *rce = NULL;

    if (strcmp(encname, "NONE") == 0) {
        original = original_stdenc_open();

        if (original == NULL)
            return EINVAL;

        return original(rce, encname, variable, lenvar);
    }

    fprintf(stderr, "STDENC: opening '%s'\n", encname);

    ce = calloc(1, sizeof(*ce));

    if (ce == NULL)
        return ENOMEM;

    ret = _citrus_load_module(&ce->ce_module, encname);

    if (ret != 0)
        goto fail;

    getops = (stdenc_getops_t)_citrus_find_getops(
        ce->ce_module, encname, "stdenc");

    if (getops == NULL) {
        ret = EINVAL;
        goto fail;
    }

    ce->ce_ops = calloc(1, sizeof(*ce->ce_ops));

    if (ce->ce_ops == NULL) {
        ret = ENOMEM;
        goto fail;
    }

    ret = getops(ce->ce_ops, sizeof(*ce->ce_ops), 2);

    if (ret != 0)
        goto fail;

    if (ce->ce_ops->eo_abi_version < 2 ||
        ce->ce_ops->eo_init == NULL ||
        ce->ce_ops->eo_uninit == NULL ||
        ce->ce_ops->eo_init_state == NULL ||
        ce->ce_ops->eo_mbtocs == NULL ||
        ce->ce_ops->eo_cstomb == NULL ||
        ce->ce_ops->eo_mbtowc == NULL ||
        ce->ce_ops->eo_wctomb == NULL ||
        ce->ce_ops->eo_get_state_desc == NULL) {
        ret = EINVAL;
        goto fail;
    }

    ce->ce_traits = calloc(1, sizeof(*ce->ce_traits));

    if (ce->ce_traits == NULL) {
        ret = ENOMEM;
        goto fail;
    }

    ret = ce->ce_ops->eo_init(
        ce, variable, lenvar, ce->ce_traits);

    if (ret != 0)
        goto fail;

    fprintf(stderr,
        "STDENC: '%s' initialized (state=%lu, max=%lu)\n",
        encname,
        (unsigned long)ce->ce_traits->et_state_size,
        (unsigned long)ce->ce_traits->et_mb_cur_max);

    *rce = ce;

    return 0;

fail:
    fprintf(stderr,
        "STDENC: '%s' initialization failed: %d\n",
        encname, ret);

    stdenc_destroy(ce, 0);

    return ret;
}
