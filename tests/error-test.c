#include <errno.h>
#include <iconv.h>
#include <stdio.h>
#include <string.h>

static int
test_invalid_encoding(void)
{
    iconv_t cd;
    int saved_errno;

    printf("\n=== Invalid encoding test ===\n");
    fflush(stdout);

    errno = 0;

    cd = iconv_open("CITRUS-NONEXISTENT", "UTF-8");
    saved_errno = errno;

    if (cd != (iconv_t)-1) {
        printf("FAIL: invalid encoding was accepted\n");
        iconv_close(cd);
        return 1;
    }

    printf("iconv_open rejected invalid encoding\n");
    printf("errno: %d (%s)\n",
        saved_errno, strerror(saved_errno));

    if (saved_errno != EINVAL) {
        printf("FAIL: expected EINVAL\n");
        return 1;
    }

    printf("PASS: invalid encoding rejected correctly\n");

    return 0;
}

static int
test_recovery(void)
{
    const char input[] = "\xe9";
    const char *inptr;
    char output[16];
    char *outptr;
    size_t inleft;
    size_t outleft;
    size_t ret;
    size_t produced;
    iconv_t cd;
    int failed;

    printf("\n=== Recovery test ===\n");
    fflush(stdout);

    cd = iconv_open("UTF-8", "ISO-8859-1");

    if (cd == (iconv_t)-1) {
        perror("FAIL: valid iconv_open");
        return 1;
    }

    memset(output, 0, sizeof(output));

    inptr = input;
    outptr = output;
    inleft = sizeof(input) - 1;
    outleft = sizeof(output);

    errno = 0;

    ret = iconv(cd, &inptr, &inleft, &outptr, &outleft);
    produced = sizeof(output) - outleft;

    failed = 0;

    if (ret == (size_t)-1) {
        perror("FAIL: valid iconv conversion");
        failed = 1;
    } else if (inleft != 0 || produced != 2 ||
        (unsigned char)output[0] != 0xc3 ||
        (unsigned char)output[1] != 0xa9) {
        printf("FAIL: unexpected conversion result\n");
        failed = 1;
    } else {
        printf("PASS: valid conversion succeeded after failure\n");
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

    printf("Citrus compatibility error-path test\n");
    fflush(stdout);

    failed = 0;

    failed |= test_invalid_encoding();
    failed |= test_recovery();

    printf("\nError-path test: %s\n",
        failed ? "FAILED" : "SUCCESS");

    return failed ? 1 : 0;
}
