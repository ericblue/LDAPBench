#include "global.h"
#include "portable.h"

#include <stdio.h>

#include <ac/stdlib.h>

#include <ac/ctype.h>
#include <ac/string.h>
#include <ac/unistd.h>
#include <ac/errno.h>
#include <sys/stat.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif

#include <ldap.h>

#include "ldif.h"
#include "lutil.h"
#include "lutil_ldap.h"
#include "ldap_defaults.h"
#include "ldap_log.h"
#include "ldap_pvt.h"

/* Function Prototypes */

static void print_entry LDAP_P((
	LDAPMessage	*entry,
	int		attrsonly));


int write_ldif LDAP_P((
	int type,
	char *name,
	char *value,
	ber_len_t vallen ));


static int ldapsearch LDAP_P((
	char	*base,
	int		scope,
	char	*filtpatt,
	char	*value,
	char	**attrs,
	int		attrsonly,
	LDAPControl **sctrls,
	LDAPControl **cctrls,
	struct timeval *timeout,
	int	sizelimit ));

int LDAPinit();
int LDAPcleanup();
int LDAPsearch (char *FILTER, char *BASE);

LDAP *ld;
