
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

struct boundary_case {
    const char *name;
    const char *bytes;
    size_t length;
    unsigned long codepoint;
    int valid;
};

static int
test_boundary(const struct boundary_case *test)
{
    mbstate_t state;
    wchar_t wc;
    size_t ret;
    size_t i;
    int saved_errno;

    memset(&state, 0, sizeof(state));
    wc = 0;
    errno = 0;

    ret = mbrtowc(&wc, test->bytes, test->length, &state);
    saved_errno = errno;

    printf("\n=== %s ===\n", test->name);
    printf("Input bytes:");

    for (i = 0; i < test->length; i++)
        printf(" %02X", (unsigned char)test->bytes[i]);

    printf("\nExpected code point: U+%08lX\n", test->codepoint);
    printf("Return: %ld\n", (long)ret);
    printf("errno: %d (%s)\n",
        saved_errno, strerror(saved_errno));

    if (ret != (size_t)-1 && ret != (size_t)-2)
        printf("Decoded value: U+%08lX\n", (unsigned long)wc);

    if (test->valid) {
        if (ret != test->length ||
            (unsigned long)wc != test->codepoint) {
            printf("FAIL: valid sequence not decoded correctly\n");
            return 1;
        }

        printf("PASS: valid sequence accepted\n");
        return 0;
    }

    if (ret != (size_t)-1 || saved_errno != EILSEQ) {
        printf("FAIL: invalid sequence was accepted\n");
        return 1;
    }

    printf("PASS: invalid sequence rejected\n");
    return 0;
}

int
main(void)
{
    static const struct boundary_case cases[] = {
        { "Highest valid Unicode code point",
          "\xF4\x8F\xBF\xBF", 4, 0x10FFFF, 1 },

        { "First code point above Unicode maximum",
          "\xF4\x90\x80\x80", 4, 0x110000, 0 },

        { "Higher four-byte sequence",
          "\xF4\xBF\xBF\xBF", 4, 0x13FFFF, 0 },

        { "Four-byte sequence beginning with F5",
          "\xF5\x80\x80\x80", 4, 0x140000, 0 },

        { "Maximum historical four-byte sequence",
          "\xF7\xBF\xBF\xBF", 4, 0x1FFFFF, 0 },

        { "Five-byte UTF-8 sequence",
          "\xF8\x88\x80\x80\x80", 5, 0x200000, 0 },

        { "Six-byte UTF-8 sequence",
          "\xFC\x84\x80\x80\x80\x80", 6, 0x4000000, 0 }
    };

    const char *locale;
    size_t i;
    int failures;

    locale = setlocale(LC_CTYPE, "en_US.UTF-8");

    if (locale == NULL) {
        fprintf(stderr, "FAIL: unable to activate UTF-8 locale\n");
        return 1;
    }

    printf("Citrus UTF-8 boundary diagnostic\n");
    printf("Locale: %s\n", locale);
    printf("MB_CUR_MAX: %lu\n", (unsigned long)MB_CUR_MAX);

    failures = 0;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
        failures += test_boundary(&cases[i]);

    printf("\nBoundary diagnostic: %d failures out of %lu cases\n",
        failures,
        (unsigned long)(sizeof(cases) / sizeof(cases[0])));

    return failures == 0 ? 0 : 1;
}
