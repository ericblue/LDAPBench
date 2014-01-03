#include "global.h"

#include <ldap.h>
#include <ldap_ssl.h>

/* Prototypes */

void tsd_setup(); 
void set_ld_error( int err, char *matched, char *errmsg, void *dummy );  
int get_ld_error( char **matched, char **errmsg, void *dummy );
void set_errno( int err );   
int get_errno( void ); 
void *  my_mutex_alloc( void );
void my_mutex_free( void *mutexp );
int LDAPinit();
int LDAPcleanup();
int LDAPsearch (char *FILTER, char *BASE);

#define errexit(code,str) \
  fprintf(stderr,"%s: %s\n",str,strerror(code)); \
  exit(1);

pthread_attr_t  attr;
LDAP *ld;
pthread_key_t  key;

struct ldap_error {
  int  le_errno;
  char  *le_matched;
  char  *le_errmsg;
};

