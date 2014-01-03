/************************************************************************
**
**  PROGRAM:      openldap-lib.c
**  DESCRIPTION:  Benchmark LDAP performance
**
**  REVISIONS:    1.0.0	06/13/99 (eb) Initial Release
**                1.1.0	02/26/01 (eb) Added OpenLDAP support
**                1.2.1 07/05/02 (eb) Minor modifications
**
**  Copyright (c) 1999-2002 Eric T. Blue <erictblue@hotmail.com>. 
**  All rights reserved.
** 
**  Redistribution and use in source and binary forms, with or without
**  modification, are permitted provided that the following conditions
**  are met:
** 
**  1. Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer. 
** 
**  2. Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following
**     disclaimer in the documentation and/or other materials
**     provided with the distribution.
** 
**  3. All advertising materials mentioning features or use of this
**     software must display the following acknowledgment:
**     "This product includes software developed by 
**      Eric T. Blue <erictblue@hotmail.com> for use in the
**      ldapbench project (http://sourceforge.net/projects/ldapbench)."
** 
**  4. The names "ldapbench" must not be used to endorse or promote
**     products derived from this software without prior written
**     permission. For written permission, please contact
**     erictblue@hotmail.com.
** 
**  5. Products derived from this software may not be called "ldapbench"
**     nor may "ldapbench" appear in their names without prior
**     written permission of Eric T. Blue.
** 
**  6. Redistributions of any form whatsoever must retain the following
**     acknowledgment:
**     "This product includes software developed by 
**      Eric T. Blue <erictblue@hotmail.com> for use in the
**      ldapbench project (http://sourceforge.net/projects/ldapbench)."
** 
**  THIS SOFTWARE IS PROVIDED BY ERIC T. BLUE ``AS IS'' AND ANY
**  EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
**  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
**  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL RALF S. ENGELSCHALL OR
**  HIS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
**  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
**  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
**  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
**  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
**  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
**  OF THE POSSIBILITY OF SUCH DAMAGE.
*************************************************************************/

#include "openldap-lib.h"


/************************************************************************
**
** FUNCTION:    LDAPsearch
**
************************************************************************/

int LDAPsearch(char *FILTER, char *BASE) {

    int rc;
    int nresponses;
    int nentries;
    int sortattr = 0;
    LDAPMessage *res, *msg;
    ber_int_t msgid;

    rc = ldap_search_ext(ld, BASE, LDAP_SCOPE_SUBTREE, FILTER, NULL, 0, 0, NULL, NULL, -1, &msgid);

    if (rc != LDAP_SUCCESS) {
	    fprintf(stderr, "%s: ldap_search_ext: %s (%d)\n",NULL, ldap_err2string(rc), rc);
	    return (rc);
    }

    nresponses = nentries = 0;

    res = NULL;

    while ((rc = ldap_result(ld, LDAP_RES_ANY,
			     sortattr ? LDAP_MSG_ALL : LDAP_MSG_ONE,
			     NULL, &res)) > 0) {

	    for (msg = ldap_first_message(ld, res); msg != NULL; msg = ldap_next_message(ld, msg)) {

	        if ((nresponses++) && (PRINT_RESULTS))
		        putchar('\n');

	        switch (ldap_msgtype(msg)) {
	            case LDAP_RES_SEARCH_ENTRY:
		            nentries++;
		            print_entry(msg, 0);
		            break;

	            case LDAP_RES_SEARCH_RESULT:
		            goto done;
	        }
	    }

	    ldap_msgfree(res);
    }

    if (rc == -1) {
	    ldap_perror(ld, "ldap_result");
	    return (rc);
    }

    done:

    return (rc);
}


/************************************************************************
**
** FUNCTION:    print_entry
**
************************************************************************/

static void print_entry(LDAPMessage * entry, int attrsonly) {

    char *a, *dn, *ufn;
    int i, rc;
    BerElement *ber = NULL;
    struct berval **bvals;
    LDAPControl **ctrls = NULL;


    dn = ldap_get_dn(ld, entry);
    ufn = NULL;

    if (PRINT_RESULTS) {
	    printf("OK!\n");
        write_ldif(LDIF_PUT_VALUE, "dn", dn, dn ? strlen(dn) : 0);
    }

    rc = ldap_get_entry_controls(ld, entry, &ctrls);

    if (rc != LDAP_SUCCESS) {
	    fprintf(stderr, "print_entry: %d\n", rc);
	    ldap_perror(ld, "ldap_get_entry_controls");
	    exit(EXIT_FAILURE);
    }

    ldap_memfree(dn);

    if (PRINT_ALL_ATTRIBS) {
	    for (a = ldap_first_attribute(ld, entry, &ber); a != NULL; a = ldap_next_attribute(ld, entry, ber)) {

	        if ((bvals = ldap_get_values_len(ld, entry, a)) != NULL) {
                for (i = 0; bvals[i] != NULL; i++) {
                    write_ldif(LDIF_PUT_VALUE, a, bvals[i]->bv_val, bvals[i]->bv_len);
		        }
		        ber_bvecfree(bvals);
            }
	    }
    }

    if (ber != NULL) {
	    ber_free(ber, 0);
    }

}

/************************************************************************
**
** FUNCTION:    write_ldif
**
************************************************************************/

int write_ldif(int type, char *name, char *value, ber_len_t vallen) {

    char *ldif;

    if ((ldif = ldif_put(type, name, value, vallen)) == NULL) {
	    return (-1);
    }

    fputs(ldif, stdout);
    ber_memfree(ldif);

    return (0);

}

/************************************************************************
**
** FUNCTION:    LDAPinit
** 
************************************************************************/

int LDAPinit() {

    int rc;

    if ((HOST != NULL || PORT) && (strlen(URI) < 2)) {

	    ld = ldap_init(HOST, PORT);
	    if (ld == NULL) {
	        perror("ldapsearch: ldap_init");
	        exit(EXIT_FAILURE);
	    }

    }

    else {
	    rc = ldap_initialize(&ld, URI);
	    if (rc != LDAP_SUCCESS) {
	        fprintf(stderr,
		        "Could not create LDAP session handle (%d): %s\n", rc, ldap_err2string(rc));
	        exit(EXIT_FAILURE);
	    }
    }

    if (strlen(BINDDN) <= 1) {
	    if (ldap_bind_s(ld, NULL, NULL, LDAP_AUTH_SIMPLE) != LDAP_SUCCESS) {
	        ldap_perror(ld, "ldap_bind");
	        exit(EXIT_FAILURE);
	    }
    }

    else {
	    if (ldap_bind_s(ld, BINDDN, BINDPW, LDAP_AUTH_SIMPLE) != LDAP_SUCCESS) {
	        ldap_perror(ld, "ldap_bind");
	        exit(EXIT_FAILURE);
	    }
    }

}

/************************************************************************
**
** FUNCTION:    LDAPcleanup
**
************************************************************************/

int LDAPcleanup(){

    ldap_unbind(ld);

}
