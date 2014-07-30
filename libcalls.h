enum libcall_refs
{
    GETPWNAM_R,
    PAM_AUTHENTICATE,
    PAM_ACCT_MGMT,
    ACCEPT,
    N_LIBCALLS    /* Number of interposed libc functions. */
};

const void *(*libcalls[N_LIBCALLS])();
