#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

struct utf8_case {
    const char *name;
    const char *bytes;
    size_t length;
    wchar_t expected;
};

static int
test_character(const struct utf8_case *test)
{
    mbstate_t state;
    wchar_t wc;
    char output[MB_LEN_MAX];
    size_t decoded;
    size_t encoded;
    size_t i;

    printf("\n=== %s ===\n", test->name);

    memset(&state, 0, sizeof(state));

    decoded = mbrtowc(&wc, test->bytes, test->length, &state);

    if (decoded == (size_t)-1 || decoded == (size_t)-2) {
        fprintf(stderr, "FAIL: mbrtowc decoding failed\n");
        return 1;
    }

    if (decoded != test->length || wc != test->expected) {
        fprintf(stderr, "FAIL: decoded character mismatch\n");
        return 1;
    }

    printf("Input bytes:");

    for (i = 0; i < test->length; i++)
        printf(" %02X", (unsigned char)test->bytes[i]);

    printf("\nDecoded character: U+%04lX\n",
        (unsigned long)wc);

    memset(&state, 0, sizeof(state));
    memset(output, 0, sizeof(output));

    encoded = wcrtomb(output, wc, &state);

    if (encoded == (size_t)-1) {
        fprintf(stderr, "FAIL: wcrtomb encoding failed\n");
        return 1;
    }

    if (encoded != test->length ||
        memcmp(output, test->bytes, test->length) != 0) {
        fprintf(stderr, "FAIL: UTF-8 round-trip mismatch\n");
        return 1;
    }

    printf("Encoded bytes:");

    for (i = 0; i < encoded; i++)
        printf(" %02X", (unsigned char)output[i]);

    printf("\nPASS: UTF-8 round trip\n");

    return 0;
}

int
main(void)
{
    static const struct utf8_case cases[] = {
        { "Two-byte UTF-8 (U+00E9)",
          "\xC3\xA9", 2, 0x00E9 },

        { "Three-byte UTF-8 (U+20AC)",
          "\xE2\x82\xAC", 3, 0x20AC },

        { "Four-byte UTF-8 (U+1F600)",
          "\xF0\x9F\x98\x80", 4, 0x1F600 }
    };

    const char *locale;
    size_t i;
    int failures;

    locale = setlocale(LC_CTYPE, "en_US.UTF-8");

    if (locale == NULL) {
        fprintf(stderr, "FAIL: unable to activate UTF-8 locale\n");
        return 1;
    }

    printf("Citrus UTF-8 demonstration\n");
    printf("Locale: %s\n", locale);
    printf("MB_CUR_MAX: %lu\n", (unsigned long)MB_CUR_MAX);

    failures = 0;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
        failures += test_character(&cases[i]);

    printf("\nUTF-8 demonstration: %s\n",
        failures == 0 ? "SUCCESS" : "FAILED");

    return failures == 0 ? 0 : 1;
}

