#include <sys/types.h>
#include <stdint.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _citrus_module_rec *_citrus_module_t;

struct _citrus_ctype;

extern struct _citrus_ctype _citrus_ctype_default;

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

struct _citrus_ctype {
    struct _citrus_ctype_ops *cc_ops;
    void *cc_closure;
    _citrus_module_t cc_module;
};

typedef int (*ctype_getops_t)(
    struct _citrus_ctype_ops *, size_t, uint32_t);

typedef int (*ctype_open_t)(
    struct _citrus_ctype **, const char *,
    void *, size_t, size_t);

extern int _citrus_load_module(_citrus_module_t *, const char *);

extern void *_citrus_find_getops(
    _citrus_module_t, const char *, const char *);

extern void _citrus_unload_module(_citrus_module_t);

static ctype_open_t
original_ctype_open(void)
{
    return (ctype_open_t)dlsym(RTLD_NEXT, "_citrus_ctype_open");
}

static void
ctype_destroy(struct _citrus_ctype *cc, int initialized)
{
    if (cc == NULL || cc == &_citrus_ctype_default)
        return;

    fprintf(stderr, "CTYPE: closing dynamic encoding\n");

    if (cc->cc_ops != NULL) {
        if (initialized && cc->cc_ops->co_uninit != NULL)
            cc->cc_ops->co_uninit(cc->cc_closure);

        free(cc->cc_ops);
    }

    if (cc->cc_module != NULL)
        _citrus_unload_module(cc->cc_module);

    free(cc);
}

void
_citrus_ctype_close(struct _citrus_ctype *cc)
{
    fprintf(stderr, "CTYPE: close called (%p)\n", (void *)cc);

    ctype_destroy(cc, 1);
}

int
_citrus_ctype_open(struct _citrus_ctype **rcc,
    const char *encname, void *variable, size_t lenvar,
    size_t szpriv)
{
    struct _citrus_ctype *cc;
    ctype_getops_t getops;
    ctype_open_t original;
    int ret;

    if (rcc == NULL || encname == NULL)
        return EINVAL;

    if (lenvar != 0 && variable == NULL)
        return EINVAL;

    *rcc = NULL;

    if (strcmp(encname, "NONE") == 0) {
        original = original_ctype_open();

        if (original == NULL)
            return EINVAL;

        return original(rcc, encname, variable, lenvar, szpriv);
    }

    fprintf(stderr, "CTYPE: opening '%s'\n", encname);

    cc = calloc(1, sizeof(*cc));

    if (cc == NULL)
        return ENOMEM;

    ret = _citrus_load_module(&cc->cc_module, encname);

    if (ret != 0)
        goto fail;

    getops = (ctype_getops_t)_citrus_find_getops(
        cc->cc_module, encname, "ctype");

    if (getops == NULL) {
        ret = EINVAL;
        goto fail;
    }

    cc->cc_ops = calloc(1, sizeof(*cc->cc_ops));

    if (cc->cc_ops == NULL) {
        ret = ENOMEM;
        goto fail;
    }

    ret = getops(cc->cc_ops, sizeof(*cc->cc_ops), 3);

    if (ret != 0)
        goto fail;

    if (cc->cc_ops->co_abi_version != 3 ||
        cc->cc_ops->co_init == NULL ||
        cc->cc_ops->co_uninit == NULL ||
        cc->cc_ops->co_get_mb_cur_max == NULL ||
        cc->cc_ops->co_mblen == NULL ||
        cc->cc_ops->co_mbrlen == NULL ||
        cc->cc_ops->co_mbrtowc == NULL ||
        cc->cc_ops->co_mbsinit == NULL ||
        cc->cc_ops->co_mbsrtowcs == NULL ||
        cc->cc_ops->co_mbstowcs == NULL ||
        cc->cc_ops->co_mbtowc == NULL ||
        cc->cc_ops->co_wcrtomb == NULL ||
        cc->cc_ops->co_wcsrtombs == NULL ||
        cc->cc_ops->co_wcstombs == NULL ||
        cc->cc_ops->co_wctomb == NULL ||
        cc->cc_ops->co_btowc == NULL ||
        cc->cc_ops->co_wctob == NULL ||
        cc->cc_ops->co_mbsnrtowcs == NULL ||
        cc->cc_ops->co_wcsnrtombs == NULL) {
        ret = EINVAL;
        goto fail;
    }

    ret = cc->cc_ops->co_init(
        &cc->cc_closure, variable, lenvar, szpriv);

    if (ret != 0)
        goto fail;

    fprintf(stderr,
        "CTYPE: '%s' initialized (private state=%lu)\n",
        encname, (unsigned long)szpriv);

    *rcc = cc;

    return 0;

fail:
    fprintf(stderr,
        "CTYPE: '%s' initialization failed: %d\n",
        encname, ret);

    ctype_destroy(cc, 0);

    return ret;
}

