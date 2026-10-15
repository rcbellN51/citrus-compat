#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

struct invalid_case {
    const char *name;
    const char *bytes;
    size_t length;
};

static int
test_invalid(const struct invalid_case *test)
{
    mbstate_t state;
    wchar_t wc;
    size_t ret;

    printf("\n=== %s ===\n", test->name);

    memset(&state, 0, sizeof(state));
    errno = 0;

    ret = mbrtowc(&wc, test->bytes, test->length, &state);

    printf("Return: %ld\n", (long)ret);
    printf("errno: %d (%s)\n", errno, strerror(errno));

    if (ret != (size_t)-1 && ret != (size_t)-2)
        printf("Decoded value: U+%08lX\n", (unsigned long)wc);

    if (ret != (size_t)-1 || errno != EILSEQ) {
        printf("FAIL: expected invalid-sequence error\n");
        return 1;
    }

    printf("PASS: invalid sequence rejected\n");
    return 0;
}

static int
test_incomplete(const struct invalid_case *test)
{
    mbstate_t state;
    wchar_t wc;
    size_t ret;

    printf("\n=== %s ===\n", test->name);

    memset(&state, 0, sizeof(state));
    errno = 0;

    ret = mbrtowc(&wc, test->bytes, test->length, &state);

    printf("Return: %ld\n", (long)ret);
    printf("errno: %d (%s)\n", errno, strerror(errno));

    if (ret != (size_t)-2) {
        printf("FAIL: expected incomplete-sequence result\n");
        return 1;
    }

    printf("PASS: incomplete sequence detected\n");
    return 0;
}

static int
test_completion(void)
{
    mbstate_t state;
    wchar_t wc;
    size_t ret;

    printf("\n=== Incremental completion ===\n");

    memset(&state, 0, sizeof(state));

    ret = mbrtowc(&wc, "\xE2", 1, &state);

    if (ret != (size_t)-2) {
        printf("FAIL: first byte not recognized as incomplete\n");
        return 1;
    }

    ret = mbrtowc(&wc, "\x82\xAC", 2, &state);

    if (ret != 2 || wc != 0x20AC) {
        printf("FAIL: incremental UTF-8 decoding failed\n");
        return 1;
    }

    printf("PASS: incremental decoding produced U+20AC\n");
    return 0;
}

static int
test_recovery(void)
{
    mbstate_t state;
    wchar_t wc;
    size_t ret;

    printf("\n=== Recovery after invalid input ===\n");

    memset(&state, 0, sizeof(state));
    errno = 0;

    ret = mbrtowc(&wc, "\xC3\x28", 2, &state);

    if (ret != (size_t)-1 || errno != EILSEQ) {
        printf("FAIL: invalid input was not rejected\n");
        return 1;
    }

    /*
     * The conversion state is unspecified after EILSEQ.
     * Reinitialize it before attempting another conversion.
     */
    memset(&state, 0, sizeof(state));

    ret = mbrtowc(&wc, "\xC3\xA9", 2, &state);

    if (ret != 2 || wc != 0x00E9) {
        printf("FAIL: decoding did not recover\n");
        return 1;
    }

    printf("PASS: fresh state decoded U+00E9\n");
    return 0;
}

int
main(void)
{
    static const struct invalid_case invalid[] = {
        { "Invalid continuation", "\xC3\x28", 2 },
        { "Stray continuation", "\x80", 1 },
        { "Overlong encoding", "\xC0\xAF", 2 },
        { "UTF-8 surrogate", "\xED\xA0\x80", 3 },
        { "Above Unicode maximum", "\xF4\x90\x80\x80", 4 }
    };

    static const struct invalid_case incomplete[] = {
        { "Incomplete two-byte sequence", "\xC3", 1 },
        { "Incomplete three-byte sequence", "\xE2\x82", 2 },
        { "Incomplete four-byte sequence", "\xF0\x9F\x98", 3 }
    };

    const char *locale;
    size_t i;
    int failures;

    locale = setlocale(LC_CTYPE, "en_US.UTF-8");

    if (locale == NULL) {
        fprintf(stderr, "FAIL: unable to activate UTF-8 locale\n");
        return 1;
    }

    printf("Citrus UTF-8 invalid-input tests\n");
    printf("Locale: %s\n", locale);

    failures = 0;

    for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++)
        failures += test_invalid(&invalid[i]);

    for (i = 0; i < sizeof(incomplete) / sizeof(incomplete[0]); i++)
        failures += test_incomplete(&incomplete[i]);

    failures += test_completion();
    failures += test_recovery();

    printf("\nUTF-8 invalid-input tests: %s\n",
        failures == 0 ? "SUCCESS" : "FAILED");

    return failures == 0 ? 0 : 1;
}
