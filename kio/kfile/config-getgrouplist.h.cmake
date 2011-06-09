#cmakedefine HAVE_GETGROUPLIST 1

/* If getgrouplist isn't provided, getgrouplist.c will provide it */
#if !defined(HAVE_GETGROUPLIST)
#include <sys/types.h> /* for gid_t */
#ifdef __cplusplus
extern "C" {
#endif
int getgrouplist(const char *, gid_t , gid_t *, int *);
#ifdef __cplusplus
}
#endif

#define HAVE_GETGROUPLIST 1
#endif

