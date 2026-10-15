#include <errno.h>
#include <iconv.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static int
test_locale(void)
{
    const char *result;
    const char input[] = "\xc3\xa9";
    mbstate_t state;
    wchar_t wc;
    size_t ret;

    printf("\n=== UTF-8 locale test ===\n");
    fflush(stdout);

    result = setlocale(LC_CTYPE, "en_US.UTF-8");

    if (result == NULL) {
        printf("FAIL: setlocale\n");
        return 1;
    }

    printf("Active locale: %s\n", result);
    printf("MB_CUR_MAX: %lu\n", (unsigned long)MB_CUR_MAX);

    if (MB_CUR_MAX < 2) {
        printf("FAIL: unexpected MB_CUR_MAX\n");
        return 1;
    }

    memset(&state, 0, sizeof(state));
    wc = 0;

    ret = mbrtowc(&wc, input, sizeof(input) - 1, &state);

    if (ret != 2 || wc != 0x00e9) {
        printf("FAIL: mbrtowc returned %lu, U+%04lx\n",
            (unsigned long)ret, (unsigned long)wc);
        return 1;
    }

    printf("PASS: decoded U+%04lx (%lu bytes)\n",
        (unsigned long)wc, (unsigned long)ret);

    return 0;
}

static int
test_iconv(void)
{
    const char input[] = "\xe9";
    char output[16];
    const char *inptr;
    char *outptr;
    size_t inleft;
    size_t outleft;
    size_t converted;
    size_t produced;
    iconv_t cd;
    int failed;

    printf("\n=== iconv conversion test ===\n");
    fflush(stdout);

    cd = iconv_open("UTF-8", "ISO-8859-1");

    if (cd == (iconv_t)-1) {
        perror("FAIL: iconv_open");
        return 1;
    }

    memset(output, 0, sizeof(output));

    inptr = input;
    outptr = output;
    inleft = sizeof(input) - 1;
    outleft = sizeof(output);

    errno = 0;
    converted = iconv(cd, &inptr, &inleft, &outptr, &outleft);
    produced = sizeof(output) - outleft;

    failed = 0;

    if (converted == (size_t)-1) {
        perror("FAIL: iconv");
        failed = 1;
    } else if (inleft != 0 || produced != 2 ||
        (unsigned char)output[0] != 0xc3 ||
        (unsigned char)output[1] != 0xa9) {
        printf("FAIL: unexpected conversion result\n");
        failed = 1;
    } else {
        printf("PASS: ISO-8859-1 E9 -> UTF-8 C3 A9\n");
    }

    if (iconv_close(cd) != 0) {
        perror("FAIL: iconv_close");
        failed = 1;
    }

    return failed;
}

int
main(void)
{
    int failed;

    failed = 0;

    printf("Citrus compatibility integration test\n");
    fflush(stdout);

    failed |= test_locale();
    failed |= test_iconv();

    printf("\n=== Locale switching ===\n");
    fflush(stdout);

    if (setlocale(LC_CTYPE, "C") == NULL) {
        printf("FAIL: switching to C locale\n");
        failed = 1;
    } else {
        printf("Active locale: %s\n",
            setlocale(LC_CTYPE, NULL));
        printf("MB_CUR_MAX: %lu\n",
            (unsigned long)MB_CUR_MAX);
    }

    failed |= test_locale();

    printf("\nIntegration test: %s\n",
        failed ? "FAILED" : "SUCCESS");

    return failed ? 1 : 0;
}
