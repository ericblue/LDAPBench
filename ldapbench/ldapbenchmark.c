/************************************************************************
**
**  PROGRAM:      ldapbenchmark.c
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

#ifdef HAVE_MOZILLA
#include "mozilla-lib.h"
#endif
#ifdef HAVE_OPENLDAP
#include "openldap-lib.h"
#endif

time_t start_time, end_time;
int total_queries = 0, curr_queries = 0;


int read_config() {

    FILE *fp;
    char line[MAX], NAME[MAX], VALUE[MAX];
    char *token;

    if ( (fp = fopen(CONFIGFILE, "r")) == NULL ) {
        printf("Can't open config file %s!\n", CONFIGFILE);
        exit(1);
    }

    while ( fgets(line, MAX, fp) != NULL ) {
        if ( line[0] == '#' )
            continue;
        if ( line[0] == '\n' )
            continue;

        if ( strstr(line, "'") ) {
            token = strtok(line, "'");
            strcpy(NAME, token);
            token = strtok(0, "'");
            strcpy(VALUE, token);
        }
        else {
            token = strtok(line, " ");
            strcpy(NAME, token);
            token = strtok(0, " ");
            strcpy(VALUE, token);
            VALUE[strlen(VALUE) - 1] = '\0';
        }

        if ( strstr(NAME, "HOST") )
            strcpy(HOST, VALUE);
        if ( strstr(NAME, "PORT") )
            PORT = atoi(VALUE);
        if ( strstr(NAME, "URI") )
            strcpy(URI, VALUE);
        if ( strstr(NAME, "QUERYFILE") )
            strcpy(QUERYFILE, VALUE);
        if ( strstr(NAME, "BINDDN") )
            strcpy(BINDDN, VALUE);
        if ( strstr(NAME, "BINDPW") )
            strcpy(BINDPW, VALUE);
        if ( strstr(NAME, "PRINT_RESULTS") )
            PRINT_RESULTS = atoi(VALUE);
        if ( strstr(NAME, "PRINT_ALL_ATTRIBS") )
            PRINT_ALL_ATTRIBS = atoi(VALUE);
        if ( strstr(NAME, "ENABLE_SSL") )
            ENABLE_SSL = atoi(VALUE);
        if ( strstr(NAME, "SSL_CERTDB") )
            strcpy(SSL_CERTDB, VALUE);
        if ( strstr(NAME, "BINDONCE") )
            BINDONCE = atoi(VALUE);
        if ( strstr(NAME, "NUM_THREADS") )
            NUM_THREADS = atoi(VALUE);
        if ( strstr(NAME, "DEBUG") )
            DEBUG = atoi(VALUE);

    }


}


Sigfunc *trap_signal(int signo, Sigfunc * func) {

    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if ( sigaction(signo, &act, &oact) < 0 )
        return(SIG_ERR);

    return(oact.sa_handler);

}

int interupt() {

    printf("Program Aborted!\n");
    cleanup();

}

int cleanup() {

    int total_time;
    float average;

    if ( BINDONCE )
        LDAPcleanup();
    time(&end_time);
    total_time = end_time - start_time;

    if ( total_time == 0 )
        total_time = 1;
    average = (curr_queries / total_time);

    printf("\nProcess Time %i seconds for %i queries: %.2f queries / second.\n",
           total_time, curr_queries, average);
    exit(0);

}

#ifdef HAVE_MOZILLA

void *doquery_threaded(void *arg) {

    char status_msg[MAX];
    int threadid = *(int *) arg;
    int i = 0;
    int thread_queries = 0;

    tsd_setup();

    /* Split up queries into chunks for each thread to handle */

    thread_queries = total_queries / NUM_THREADS;

    for ( i = ((thread_queries * threadid) - thread_queries);
        i < (thread_queries * threadid); i++ ) {
        /* printf("%i: %s\n",i,queries[i].FILTER); */
        LDAPsearch(queries[i].FILTER, queries[i].BASE);
    }

    printf("Thread %i completed %i queries\n", threadid, thread_queries);
    return(arg);

}
#endif

int doquery() {

    char status_msg[MAX];
    int i = 0;

    for ( i = 0; i < total_queries; i++ ) {
        if ( !(BINDONCE) )
            LDAPinit();

        printf("%i: %s\n",i,queries[i].FILTER); 
        LDAPsearch(queries[i].FILTER, queries[i].BASE);
        curr_queries++;

        if ( !(BINDONCE) )
            LDAPcleanup();

        if ( !(i % 1000) && (i != 0) ) {
            sprintf(status_msg, "Completed %i queries.", i);
            printf("%s\n", status_msg);
        }
    }


}

int load_queries() {

    FILE *fp;
    char line[MAX];
    char *token;

    if ( (fp = fopen(QUERYFILE, "r")) == NULL ) {
        printf("Can't open query file %s!\n", QUERYFILE);
        exit(1);
    }

    total_queries = 0;
    while ( fgets(line, MAX, fp) != NULL ) {
        line[strlen(line) - 1] = '\0';
        if ( !(strstr(line, " ")) )
            continue;
        token = strtok(line, " ");
        sprintf(queries[total_queries].FILTER, "%s", token);

        token = strtok(0, " ");
        sprintf(queries[total_queries].BASE, "%s", token);
        total_queries++;
    }

    fclose(fp);

}


/************************************************************************
**
** FUNCTION:    main
** 
************************************************************************/

int main(int argc, char *argv[]) {

    int worker;
    pthread_t threads[MAX_THREADS];
    int ids[MAX_THREADS];
    int errcode;
    int *status;
    char query_type[MAX];

    if ( !(argv[1]) ) {
        printf("Usage: %s <CONFIGFILE>\n", argv[0]);
        exit(1);
    }

    strcpy(CONFIGFILE, argv[1]);

    read_config();

    if ( argv[2] ) {
        if ( strcmp(argv[2], "-v") == 0 ) {
            PRINT_RESULTS = 1;
            PRINT_ALL_ATTRIBS = 1;
            DEBUG = 1;
        }
    }

    printf("Print results = %i\n", PRINT_RESULTS);

    load_queries();
    time(&start_time);

    if ( BINDONCE ) {
        strcpy(query_type, "PERSISTENT");
    }
    else {
        strcpy(query_type, "NON-PERSISTENT");
    }


    printf("Performing %s queries from file %s.  Please Wait.\n\n",
           query_type, QUERYFILE);
    if ( BINDONCE )
        LDAPinit();

    if ( NUM_THREADS == 0 ) {
        trap_signal(SIGINT, (Sigfunc *) interupt);
        doquery();
    }
#ifdef HAVE_MOZILLA
    else {
        for ( worker = 1; worker <= NUM_THREADS; worker++ ) {
            ids[worker] = worker;
            if ( errcode = pthread_create(&threads[worker], &attr, doquery_threaded, &ids[worker]) ) {
                errexit(errcode, "pthread_create");
            }
        }

        for ( worker = 1; worker <= NUM_THREADS; worker++ ) {
            if ( errcode = pthread_join(threads[worker], (void *) &status) ) {
                errexit(errcode, "pthread_join");
            }
            if ( *status != worker ) {
                fprintf(stderr, "thread %d terminated abnormally\n", worker);
                exit(1);
            }
        }

        curr_queries = total_queries;
    }
#endif

    cleanup();

}