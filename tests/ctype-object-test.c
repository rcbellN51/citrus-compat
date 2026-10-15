#include <stdint.h>
#include <stdio.h>

extern unsigned char _citrus_ctype_default[];
extern unsigned char _citrus_NONE_ctype_ops[];

int
main(void)
{
    const uint32_t *object;
    size_t i;

    object = (const uint32_t *)(const void *)
        _citrus_ctype_default;

    printf("Default CTYPE object: %p\n",
        (void *)_citrus_ctype_default);

    printf("NONE operations table: %p\n",
        (void *)_citrus_NONE_ctype_ops);

    for (i = 0; i < 3; i++) {
        printf("Object[%lu]: 0x%08lx\n",
            (unsigned long)i,
            (unsigned long)object[i]);
    }

    return 0;
}
