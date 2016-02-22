#ifndef H0H0_H
#define H0H0_H

#include <stdio.h>

/* Debug output. */
#ifdef DEBUG
#define debug(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
    #define debug(fmt, ...) /* Not debugging. */
#endif

#endif // H0H0_H
