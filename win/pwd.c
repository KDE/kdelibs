
#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <errno.h>

/* these functions always fail. for win32 */

KDEWIN32_EXPORT struct passwd * getpwnam (const char *name)
{
	return 0;
}

KDEWIN32_EXPORT 
struct passwd * getpwuid (uid_t uid)
{
	return 0;
}

KDEWIN32_EXPORT 
void setpwent (void)
{
}

KDEWIN32_EXPORT 
struct passwd	*getpwent()
{
	return 0;
}

KDEWIN32_EXPORT 
void endpwent()
{
}

