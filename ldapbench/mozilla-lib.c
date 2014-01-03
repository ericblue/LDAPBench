/************************************************************************
**
**  PROGRAM:      mozilla-lib.c
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

#include "mozilla-lib.h"

/************************************************************************
**
** FUNCTION:    tsd_setup
** DESCRIPTION: Setup thread specific data
** 
************************************************************************/
void tsd_setup() {

    void *tsd;

    tsd = pthread_getspecific(key);
    if ( tsd != NULL ) {
        fprintf(stderr, "tsd non-null!\n");
        pthread_exit(NULL);
    }

    tsd = (void *) calloc(1, sizeof(struct ldap_error));
    pthread_setspecific(key, tsd);

}

/************************************************************************
**
** FUNCTION:    set_ld_error
** 
************************************************************************/

void set_ld_error(int err, char *matched, char *errmsg, void *dummy) {

    struct ldap_error *le;

    le = pthread_getspecific(key);
    le->le_errno = err;
    if ( le->le_matched != NULL ) {
        ldap_memfree(le->le_matched);
    }
    le->le_matched = matched;
    if ( le->le_errmsg != NULL ) {
        ldap_memfree(le->le_errmsg);
    }
    le->le_errmsg = errmsg;
}

/************************************************************************
**
** FUNCTION:    get_ld_error
** 
************************************************************************/
int get_ld_error(char **matched, char **errmsg, void *dummy) {

    struct ldap_error *le;

    le = pthread_getspecific(key);
    if ( matched != NULL ) {
        *matched = le->le_matched;
    }
    if ( errmsg != NULL ) {
        *errmsg = le->le_errmsg;
    }

    return(le->le_errno);

}

/************************************************************************
**
** FUNCTION:    set_error_no
** 
************************************************************************/
void set_errno(int err) {

    errno = err;

}

/************************************************************************
**
** FUNCTION:    get_error_no
** 
************************************************************************/
int get_errno(void) {

    return(errno);

}

/************************************************************************
**
** FUNCTION:    my_mutex_alloc
** 
************************************************************************/
void *my_mutex_alloc(void) {

    pthread_mutex_t *mutexp;
    if ( (mutexp = malloc(sizeof(pthread_mutex_t))) != NULL ) {
        pthread_mutex_init(mutexp, NULL);
    }

    return(mutexp);

}

/************************************************************************
**
** FUNCTION:    my_mutex_free
** 
************************************************************************/
void my_mutex_free(void *mutexp) {

    pthread_mutex_destroy((pthread_mutex_t *) mutexp);
    free(mutexp);

}

/************************************************************************
**
** FUNCTION:    LDAPinit
** 
************************************************************************/

int LDAPinit() {

    int rc;
    struct ldap_thread_fns tfns;
    struct ldap_extra_thread_fns extrafns;


    if ( ENABLE_SSL ) {
        if ( ldapssl_client_init(SSL_CERTDB, NULL) < 0 ) {
            perror("ldapssl_client_init");
            exit(1);
        }

        if ( (ld = ldapssl_init(HOST, PORT, 1)) == NULL ) {
            perror("ldapssl_init");
            exit(1);
        }

    }

    else {
        if ( (ld = ldap_init(HOST, PORT)) == NULL ) {
            perror("ldap_init");
            exit(1);
        }
    }

    if ( NUM_THREADS > 0 ) {

        if ( pthread_key_create(&key, free) != 0 ) {
            perror("pthread_key_create");
            exit(1);
        }

        tsd_setup();

        /* Set the function pointers for dealing with mutexes and error information. */

        memset(&tfns, '\0', sizeof(struct ldap_thread_fns));
        tfns.ltf_mutex_alloc = (void *(*)(void)) my_mutex_alloc;
        tfns.ltf_mutex_free = (void (*)(void *)) my_mutex_free;
        tfns.ltf_mutex_lock = (int (*)(void *)) pthread_mutex_lock;
        tfns.ltf_mutex_unlock = (int (*)(void *)) pthread_mutex_unlock;
        tfns.ltf_get_errno = get_errno;
        tfns.ltf_set_errno = set_errno;
        tfns.ltf_get_lderrno = get_ld_error;
        tfns.ltf_set_lderrno = set_ld_error;
        tfns.ltf_lderrno_arg = NULL;

        /* Set up this session to use those function pointers. */
        rc = ldap_set_option(ld, LDAP_OPT_THREAD_FN_PTRS, (void *) &tfns);
        if ( rc < 0 ) {
            fprintf(stderr,
                    "ldap_set_option (LDAP_OPT_THREAD_FN_PTRS): %s\n",
                    ldap_err2string(rc));
            exit(1);
        }

        /* Set the function pointers for working with semaphores. */
        memset(&extrafns, '\0', sizeof(struct ldap_extra_thread_fns));
        extrafns.ltf_mutex_trylock = (int (*)(void *)) NULL;
        extrafns.ltf_sema_alloc = (void *(*)(void)) NULL;
        extrafns.ltf_sema_free = (void (*)(void *)) NULL;
        extrafns.ltf_sema_wait = (int (*)(void *)) NULL;
        extrafns.ltf_sema_post = (int (*)(void *)) NULL;
        extrafns.ltf_threadid_fn = (void *(*)(void)) pthread_self;

        /* Set up this session to use those function pointers. */
        rc = ldap_set_option(ld, LDAP_OPT_EXTRA_THREAD_FN_PTRS,
                             (void *) &extrafns);
        if ( rc < 0 ) {
            fprintf(stderr,
                    "ldap_set_option (LDAP_OPT_EXTRA_THREAD_FN_PTRS): %s\n",
                    ldap_err2string(rc));
            exit(1);
        }

        /* Initialize the attribute. */
        if ( pthread_attr_init(&attr) != 0 ) {
            perror("pthread_attr_init");
            exit(1);
        }

        /* Specify that the threads are joinable. */
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    }

    if ( strlen(BINDDN) <= 1 ) {

        if ( ldap_simple_bind_s(ld, NULL, NULL) != LDAP_SUCCESS ) {
            ldap_perror(ld, "ldap_simple_bind_s (anon)");
            ldap_unbind(ld);
            exit(1);
        }
    }

    else {
        if ( ldap_simple_bind_s(ld, BINDDN, BINDPW) != LDAP_SUCCESS ) {
            ldap_perror(ld, "ldap_simple_bind_s (std)");
            ldap_unbind(ld);
            exit(1);
        }
    }

}

/************************************************************************
**
** FUNCTION:    LDAPcleanup
** 
************************************************************************/

int LDAPcleanup() {

    ldap_unbind(ld);

}

/************************************************************************
**
** FUNCTION:    LDAPsearch
** 
************************************************************************/

int LDAPsearch(char *FILTER, char *BASE) {

    LDAPMessage *result, *e;
    BerElement *ber;
    char *attr, *dn;
    char **vals;
    int i;


    if ( ldap_search_s(ld, BASE, LDAP_SCOPE_SUBTREE, FILTER, NULL, 0,
                       &result) != LDAP_SUCCESS ) {
        if ( DEBUG )
            ldap_perror(ld, "ldap_search_s");
        if ( result == NULL ) {
            ldap_unbind(ld);
            exit(1);
        }
    }

/* for each entry print out name + all attrs and values */
    for ( e = ldap_first_entry(ld, result); e != NULL; e = ldap_next_entry(ld, e) ) {
        if ( (dn = ldap_get_dn(ld, e)) != NULL ) {
            if ( PRINT_RESULTS )
                printf("\ndn: %s\n", dn);
            ldap_memfree(dn);
        }

        if ( PRINT_ALL_ATTRIBS ) {

            for ( attr = ldap_first_attribute(ld, e, &ber); attr != NULL; attr = ldap_next_attribute(ld, e, ber) ) {
                if ( (vals = ldap_get_values(ld, e, attr)) != NULL ) {
                    for ( i = 0; vals[i] != NULL; i++ ) {
                        if ( PRINT_RESULTS )
                            printf("%s: %s\n", attr, vals[i]);
                    }
                    ldap_value_free(vals);
                }
                ldap_memfree(attr);
            }

            if ( ber != NULL )
                ber_free(ber, 0);
        }

    }

    ldap_msgfree(result);

}