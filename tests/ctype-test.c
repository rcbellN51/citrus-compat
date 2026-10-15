#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

struct _citrus_ctype;

struct _citrus_ctype_ops {
    uint32_t co_abi_version;
    int (*co_init)(void **, void *, size_t, size_t);
    void (*co_uninit)(void *);
    unsigned (*co_get_mb_cur_max)(void *);
};

struct _citrus_ctype {
    struct _citrus_ctype_ops *cc_ops;
    void *cc_closure;
    void *cc_module;
};

extern int _citrus_ctype_open(struct _citrus_ctype **,
    const char *, void *, size_t, size_t);

extern void _citrus_ctype_close(struct _citrus_ctype *);

int
main(void)
{
    struct _citrus_ctype *cc;
    unsigned max;
    int ret;

    cc = NULL;

    ret = _citrus_ctype_open(&cc, "UTF8", NULL, 0, 112);

    printf("CTYPE open result: %d\n", ret);

    if (ret != 0)
        return 1;

    printf("CTYPE object: %p\n", (void *)cc);
    printf("CTYPE ABI version: %lu\n",
        (unsigned long)cc->cc_ops->co_abi_version);

    max = cc->cc_ops->co_get_mb_cur_max(cc->cc_closure);

    printf("CTYPE maximum character length: %u\n", max);

    _citrus_ctype_close(cc);

    return 0;
}

