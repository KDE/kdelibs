#ifndef __kssldefs_h
#define __kssldefs_h

#include "ksslconfig.h"
#ifdef HAVE_OLD_SSL_API
#define OPENSSL_malloc Malloc
#define OPENSSL_malloc_locked Malloc_locked
#define OPENSSL_realloc Realloc
#define OPENSSL_free Free
#endif

#endif
