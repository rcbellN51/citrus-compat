#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static int
test_locale(const char *name, const char *input,
    size_t length, wchar_t expected)
{
    const char *result;
    mbstate_t state;
    wchar_t wc;
    size_t ret;

    printf("\nSwitching to %s\n", name);
    fflush(stdout);

    result = setlocale(LC_CTYPE, name);

    if (result == NULL) {
        printf("setlocale failed\n");
        return 1;
    }

    printf("Active locale: %s\n", result);
    printf("MB_CUR_MAX: %lu\n", (unsigned long)MB_CUR_MAX);

    memset(&state, 0, sizeof(state));

    ret = mbrtowc(&wc, input, length, &state);

    if (ret == (size_t)-1 || ret == (size_t)-2) {
        printf("mbrtowc failed: %lu\n", (unsigned long)ret);
        return 1;
    }

    printf("Decoded: U+%04lx (%lu bytes)\n",
        (unsigned long)wc, (unsigned long)ret);

    if (wc != expected) {
        printf("Unexpected character\n");
        return 1;
    }

    return 0;
}

int
main(void)
{
    int failed;

    failed = 0;

    failed |= test_locale("C", "A", 1, L'A');

    failed |= test_locale("en_US.UTF-8",
        "\xc3\xa9", 2, 0x00e9);

    failed |= test_locale("en_US.ISO8859-1",
        "\xe9", 1, 0x00e9);

    failed |= test_locale("C", "A", 1, L'A');

    failed |= test_locale("en_US.UTF-8",
        "\xc3\xa9", 2, 0x00e9);

    failed |= test_locale("C", "A", 1, L'A');

    printf("\nLocale switching: %s\n",
        failed ? "FAILED" : "SUCCESS");

    return failed ? 1 : 0;
}

