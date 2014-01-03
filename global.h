#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <malloc.h>
#include <time.h>
#include <errno.h>

typedef void    Sigfunc(int);

#define MAX 255
#define MAX_THREADS 64

/* Globals */
  
char    CONFIGFILE[64];
int     NUM_THREADS;
char    HOST[64];
int     PORT;
char	URI[64];
char    BINDDN[64];
char    BINDPW[64];
int     PRINT_RESULTS;
int     PRINT_ALL_ATTRIBS;
char    QUERYFILE[64];
int     ENABLE_SSL;
char    SSL_CERTDB[64];
int     BINDONCE; 
int     DEBUG;

struct query {
  char FILTER[MAX];
  char BASE[MAX];
};

struct query queries[100000];
