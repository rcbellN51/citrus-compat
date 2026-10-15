#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

int
main(void)
{
    const char *locale;
    const char *input;
    mbstate_t state;
    wchar_t wc;
    size_t ret;

    locale = setlocale(LC_CTYPE, "en_US.UTF-8");

    if (locale == NULL) {
        fprintf(stderr, "Unable to activate UTF-8 locale\n");
        return 1;
    }

    printf("Locale: %s\n", locale);
    printf("MB_CUR_MAX: %lu\n", (unsigned long)MB_CUR_MAX);

    input = "\xc3\xa9";

    memset(&state, 0, sizeof(state));

    ret = mbrtowc(&wc, input, strlen(input), &state);

    if (ret == (size_t)-1 || ret == (size_t)-2) {
        fprintf(stderr, "UTF-8 decoding failed\n");
        return 1;
    }

    printf("Input bytes: C3 A9\n");
    printf("Decoded character: U+%04lx\n",
        (unsigned long)wc);

    printf("Wide character classification: %s\n",
        iswalpha(wc) ? "alphabetic" : "not alphabetic");

    printf("UTF-8 output: %s\n", input);

    return 0;
}
