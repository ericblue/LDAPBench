#
# LDAPBenchmark Makefile v1.2 - 07/02/01 (etb)
#

# Linux
OTHERLIB = -lpthread
OSTYPE = linux

# Solaris
#OTHERLIB = -lsocket -lnsl -lpthread
#OSTYPE = solaris

# OpenLDAP SDK
SDK = openldap
LDAPLIB = -lldap -llutil -llber -lcrypt -lresolv -ldl
DEFINES = -DHAVE_OPENLDAP
SDK_DIR = /usr/src/openldap-2.2.17

# Mozilla SDK
#SDK = mozilla
#LDAPLIB = -lldapssl41
#DEFINES = -DHAVE_MOZILLA

INCLUDE = -I$(SDK_DIR)/include
LIBRARIES = -L$(SDK_DIR)/lib/


CC = gcc -g -O2
LIBS =$(LDAPLIB) $(OTHERLIB) 
ARGS = $(INCLUDE) $(LIBRARIES) $(DEFINES)

OBJS = $(SDK)-lib.o
PROGS=ldapbenchmark
OUT = ldapbenchmark

all:		$(OBJS) $(PROGS)

ldapbenchmark:	ldapbenchmark.o
		$(CC) $(ARGS) -o $(OUT) ldapbenchmark.o $(SDK)-lib.o $(LIBS)

ldapbenchmark.o:  ldapbenchmark.c 
		$(CC) $(ARGS) -c ldapbenchmark.c -o ldapbenchmark.o

$(SDK)-lib.o:  $(SDK)-lib.c 
		$(CC) $(ARGS) -c $(SDK)-lib.c

clean:
		/bin/rm -f $(PROGS) *.o a.out core

