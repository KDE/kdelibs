# Thomas Nagy 2005

def detect(env,dest):
	import os, re

	content += """
#define HAVE_BOOL 1
#define HAVE_CUPS 1
#define HAVE_CUPS_NO_PWD_CACHE 1
#define HAVE_DLERROR 1
#define HAVE_DNSSD 1
#define HAVE_EXR 1
#define HAVE_FUNC_FINITE 1
#define HAVE_FUNC_ISINF 1
#define HAVE_FUNC_ISNAN 1
#define HAVE_GETCWD 1
#define HAVE_GETMNTINFO 1
#define HAVE_GETPAGESIZE 1
#define HAVE_LC_MESSAGES 1
#define HAVE_LIBDL 1
#define HAVE_LIBGSSAPI 1
#define HAVE_LIBPTHREAD 1
#define HAVE_MEMCPY 1
#define HAVE_MITSHM 1
#define HAVE_MMAP 1
#define HAVE_MUNMAP 1
#define HAVE_PRELOADED_SYMBOLS 1
#define HAVE_PUTENV 1
#define HAVE_RES_INIT 1
#define HAVE_RES_INIT_PROTO 1
#define HAVE_REVOKE 1
#define HAVE_SETLOCALE 1
#define HAVE_SGI_STL 1
#define HAVE_SNPRINTF 1
#define HAVE_STRCASECMP 1
#define HAVE_STRCHR 1
#define HAVE_STRCMP 1
#define HAVE_STRRCHR 1
#define HAVE_STRUCT_ADDRINFO 1
#define HAVE_STRUCT_SOCKADDR_IN6 1
#define HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID 1
#define HAVE_STRUCT_SOCKADDR_SA_LEN 1
/* #undef HAVE_UNLOCKPT */
/* #undef HAVE_UTEMPTER */
#define HAVE_VSNPRINTF 1

/* Define to 1 if the assembler supports 3DNOW instructions. */
/* #undef HAVE_X86_3DNOW */

/* Define to 1 if the assembler supports MMX instructions. */
/* #undef HAVE_X86_MMX */

/* Define to 1 if the assembler supports SSE instructions. */
/* #undef HAVE_X86_SSE */

/* Define to 1 if the assembler supports SSE2 instructions. */
/* #undef HAVE_X86_SSE2 */

/* Defined if your system has XRender support */
#define HAVE_XRENDER 1

/* Define to 1 if you have the `_getpty' function. */
/* #undef HAVE__GETPTY */

/* Define to 1 if you have the `_IceTransNoListen' function. */
#define HAVE__ICETRANSNOLISTEN 1

/* Define to 1 if you have the `__argz_count' function. */
/* #undef HAVE___ARGZ_COUNT */

/* Define to 1 if you have the `__argz_next' function. */
/* #undef HAVE___ARGZ_NEXT */

/* Define to 1 if you have the `__argz_stringify' function. */
/* #undef HAVE___ARGZ_STRINGIFY */

/* The prefix to use as fallback */
#define KDEDIR "/sw"

/* Use FontConfig in kdeinit */
#define KDEINIT_USE_FONTCONFIG 1

/* Use Xft preinitialization in kdeinit */
/* #undef KDEINIT_USE_XFT */

/* The compiled in system configuration prefix */
#define KDESYSCONFDIR "/sw/etc"

/* Distribution Text to append to OS */
#define KDE_DISTRIBUTION_TEXT "Fink/Mac OS X"

/* Use own malloc implementation */
/* #undef KDE_MALLOC */

/* Enable debugging in fast malloc */
/* #undef KDE_MALLOC_DEBUG */

/* Make alloc as fast as possible */
/* #undef KDE_MALLOC_FULL */

/* The libc used is glibc */
/* #undef KDE_MALLOC_GLIBC */

/* The platform is x86 */
/* #undef KDE_MALLOC_X86 */

/* Define if we shall use KSSL */
#define KSSL_HAVE_SSL 1
#define NEED_BZ2_PREFIX 1
#define SIZEOF_CHAR_P 4
#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define SIZEOF_SHORT 2
#define SIZEOF_SIZE_T 4
#define SIZEOF_UNSIGNED_LONG 4
#define STDC_HEADERS 1
#define TIME_WITH_SYS_TIME 1
#define WORDS_BIGENDIAN 1

/* where rgb.txt is in */
#define X11_RGBFILE "/usr/X11R6/lib/X11/rgb.txt"

/* Defines the executable of xmllint */
#define XMLLINT "/sw/bin/xmllint"

#ifdef HAVE__NSGETENVIRON
/* backwards-compat with existing code :P */
#define HAVE_NSGETENVIRON
#endif

#if defined(HAVE_NSGETENVIRON) && defined(HAVE_CRT_EXTERNS_H)
# include <sys/time.h>
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#endif


#if !defined(HAVE_RES_INIT_PROTO)
#ifdef __cplusplus
extern "C" {
#endif
int res_init(void);
#ifdef __cplusplus
}
#endif
#endif

#define HAVE_S_ISSOCK 1

#if !defined(HAVE_VSNPRINTF) || defined(hpux)
#if __STDC__
#include <stdarg.h>
#include <stdlib.h>
#else
#include <varargs.h>
#endif
#ifdef __cplusplus
extern "C"
#endif
int vsnprintf(char *str, size_t n, char const *fmt, va_list ap);
#ifdef __cplusplus
extern "C"
#endif
int snprintf(char *str, size_t n, char const *fmt, ...);
#endif


/* KDE bindir */
#define __KDE_BINDIR "/sw/bin"

/* execprefix or NONE if not set, for libloading */
#define __KDE_EXECPREFIX "NONE"

/* define to 1 if -fvisibility is supported */
/* #undef __KDE_HAVE_GCC_VISIBILITY */

/* path to su */
#define __PATH_SU "/sw/bin/su"


#if defined(__SVR4) && !defined(__svr4__)
#define __svr4__ 1
#endif

#define kde_socklen_t socklen_t
#define ksize_t socklen_t

#define LTDL_OBJDIR ".libs/"
#define LTDL_SHLIBPATH_VAR "DYLD_LIBRARY_PATH"
#define LTDL_SYSSEARCHPATH "/lib:/usr/lib"
"""

	## commented out for now; the real fix is not the above, but to write real tests in lowlevel.py
	dest.write(content)

def define_line(define, bool):

	if bool:
		return "#define " + define + " 1\n"
	else:
		return "/* #undef " + define + " */\n"

