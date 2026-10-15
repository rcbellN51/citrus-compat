#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>

int
main(void)
{
    const char input[] = "\351";
    char output[16];
    const char *inptr;
    char *outptr;
    size_t inleft;
    size_t outleft;
    size_t converted;
    size_t produced;
    iconv_t cd;

    cd = iconv_open("UTF-8", "ISO-8859-1");

    if (cd == (iconv_t)-1) {
        perror("iconv_open");
        return EXIT_FAILURE;
    }

    memset(output, 0, sizeof(output));

    inptr = input;
    outptr = output;
    inleft = sizeof(input) - 1;
    outleft = sizeof(output);

    converted = iconv(cd, &inptr, &inleft, &outptr, &outleft);

    if (converted == (size_t)-1) {
        perror("iconv");
        iconv_close(cd);
        return EXIT_FAILURE;
    }

    produced = sizeof(output) - outleft;

    printf("Input:  E9\n");
    printf("Output:");

    for (size_t i = 0; i < produced; i++)
        printf(" %02X", (unsigned char)output[i]);

    printf("\n");

    iconv_close(cd);

    if (produced != 2 ||
        (unsigned char)output[0] != 0xC3 ||
        (unsigned char)output[1] != 0xA9)
        return EXIT_FAILURE;

    printf("UTF-8 conversion successful.\n");

    return EXIT_SUCCESS;
}
