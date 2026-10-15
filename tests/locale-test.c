
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

int
main(void)
{
    const char *locale;
    wchar_t wc;
    mbstate_t state;
    size_t ret;

    locale = setlocale(LC_CTYPE, "en_US.UTF-8");

    printf("LC_CTYPE: %s\n",
        locale != NULL ? locale : "(null)");

    if (locale == NULL)
        return 1;

    printf("MB_CUR_MAX: %lu\n",
        (unsigned long)MB_CUR_MAX);

    state = (mbstate_t){0};

    ret = mbrtowc(&wc, "\xc3\xa9", 2, &state);

    printf("mbrtowc result: %lu\n",
        (unsigned long)ret);

    if (ret == (size_t)-1 || ret == (size_t)-2)
        return 1;

    printf("Unicode code point: U+%04lx\n",
        (unsigned long)wc);

    return wc == 0x00e9 ? 0 : 1;
}

