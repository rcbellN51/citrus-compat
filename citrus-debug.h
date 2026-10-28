#ifndef CITRUS_DEBUG_H
#define CITRUS_DEBUG_H

#ifdef CITRUS_DEBUG
#include <stdio.h>
#define CITRUS_LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define CITRUS_LOG(...) ((void)0)
#endif

#endif
