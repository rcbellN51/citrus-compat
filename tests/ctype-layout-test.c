#include <stdint.h>
#include <stdio.h>

extern unsigned char _citrus_ctype_default[];

int
main(void)
{
    const uint32_t *words;
    size_t i;

    words = (const uint32_t *)(const void *)_citrus_ctype_default;

    for (i = 0; i < 3; i++) {
        printf("CTYPE default word %lu: 0x%08lx\n",
            (unsigned long)i,
            (unsigned long)words[i]);
    }

    return 0;
}
