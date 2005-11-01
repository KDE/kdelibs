# Thomas Nagy 2005

def detect(lenv,dest):
	import os

	content="""
/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1
#define HAVE_ALLOCA 1
#define HAVE_ALLOCA_H 1
#define HAVE_ARPA_NAMESER8_COMPAT_H 1
#define HAVE_BOOL 1
#define HAVE_BZIP2_SUPPORT 1
#define HAVE_CARBON_CARBON_H 1
#define HAVE_COREAUDIO 1
#define HAVE_CRT_EXTERNS_H 1
#define HAVE_CRYPT 1
#define HAVE_CTYPE_H 1
#define HAVE_CUPS 1
#define HAVE_CUPS_NO_PWD_CACHE 1
#define HAVE_DIRENT_H 1
#define HAVE_DLERROR 1
#define HAVE_DLFCN_H 1
#define HAVE_DNSSD 1
#define HAVE_EXR 1
#define HAVE_FLOAT_H 1
#define HAVE_FREEADDRINFO 1
#define HAVE_FSTAB_H 1
#define HAVE_FUNC_FINITE 1
#define HAVE_FUNC_ISINF 1
#define HAVE_FUNC_ISNAN 1
#define HAVE_GAI_STRERROR 1
#define HAVE_GETADDRINFO 1
#define HAVE_GETCWD 1
#define HAVE_GETGROUPS 1
#define HAVE_GETHOSTBYNAME2 1

/* Define to 1 if you have the `gethostbyname2_r' function. */
/* #undef HAVE_GETHOSTBYNAME2_R */

/* Define to 1 if you have the `gethostbyname_r' function. */
/* #undef HAVE_GETHOSTBYNAME_R */

/* Define to 1 if you have the `getmntinfo' function. */
#define HAVE_GETMNTINFO 1
#define HAVE_GETNAMEINFO 1
#define HAVE_GETPAGESIZE 1
#define HAVE_GETPEEREID 1
#define HAVE_GETPEERNAME 1
#define HAVE_GETSOCKNAME 1
#define HAVE_GETSOCKOPT 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_GRANTPT 1
#define HAVE_IDNA_H 1
#define HAVE_IF_NAMETOINDEX 1
#define HAVE_INET_NTOP 1
#define HAVE_INET_PTON 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1
#define HAVE_LC_MESSAGES 1
#define HAVE_LIBART 1
#define HAVE_LIBDL 1
#define HAVE_LIBGSSAPI 1
#define HAVE_LIBIDN 1
#define HAVE_LIBJPEG 1
#define HAVE_LIBPNG 1
#define HAVE_LIBPTHREAD 1
#define HAVE_LIBTIFF 1
#define HAVE_LIBZ 1
#define HAVE_LIMITS_H 1
#define HAVE_LOCALE_H 1
#define HAVE_MEMCPY 1
#define HAVE_MEMORY_H 1
#define HAVE_MITSHM 1

/* Define to 1 if you have a working `mmap' system call. */
#define HAVE_MMAP 1
#define HAVE_MUNMAP 1
#define HAVE_NETINET_IN_H 1
#define HAVE_NET_IF_H 1
#define HAVE_NL_TYPES_H 1
#define HAVE_NSGETENVIRON 1
#define HAVE_OPENPTY 1
#define HAVE_PATHS_H 1
#define HAVE_PCREPOSIX 1
#define HAVE_POLL 1
#define HAVE_PRELOADED_SYMBOLS 1
/* #undef HAVE_PTSNAME */
#define HAVE_PUNYCODE_H 1
#define HAVE_PUTENV 1

/* Define if you have res_init */
#define HAVE_RES_INIT 1
#define HAVE_RES_INIT_PROTO 1
#define HAVE_REVOKE 1
#define HAVE_SETEGID 1

/* Define to 1 if you have the `seteuid' function. */
#define HAVE_SETEUID 1
#define HAVE_SETFSENT 1
#define HAVE_SETGROUPS 1
#define HAVE_SETLOCALE 1
#define HAVE_SETPRIORITY 1
#define HAVE_SGI_STL 1
#define HAVE_SNPRINTF 1
#define HAVE_SOCKET 1

/* If we are going to use OpenSSL */
#define HAVE_SSL 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STPCPY 1
#define HAVE_STRCASECMP 1
#define HAVE_STRCHR 1
#define HAVE_STRCMP 1
#define HAVE_STRFMON 1
#define HAVE_STRINGPREP_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strrchr' function. */
#define HAVE_STRRCHR 1
#define HAVE_STRTOLL 1
#define HAVE_STRUCT_ADDRINFO 1
#define HAVE_STRUCT_SOCKADDR_IN6 1
#define HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID 1
#define HAVE_STRUCT_SOCKADDR_SA_LEN 1
#define HAVE_SYS_FILIO_H 1
#define HAVE_SYS_MMAN_H 1
#define HAVE_SYS_MOUNT_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_UCRED_H 1
#define HAVE_S_ISSOCK 1
#define HAVE_TERMIOS_H 1
#define HAVE_UNISTD_H 1
/* #undef HAVE_UNLOCKPT */

/* Define if you have the utempter helper for utmp managment */
/* #undef HAVE_UTEMPTER */

/* Define to 1 if you have the <util.h> header file. */
#define HAVE_UTIL_H 1
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the <X11/extensions/shape.h> header file. */
#define HAVE_X11_EXTENSIONS_SHAPE_H 1

/* Define to 1 if you have the <X11/extensions/XShm.h> header file. */
#define HAVE_X11_EXTENSIONS_XSHM_H 1

/* Define to 1 if you have the <X11/ICE/ICElib.h> header file. */
#define HAVE_X11_ICE_ICELIB_H 1

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

/* Define to 1 if you have the </usr/src/sys/gnu/i386/isa/sound/awe_voice.h>
   header file. */
/* #undef HAVE__USR_SRC_SYS_GNU_I386_ISA_SOUND_AWE_VOICE_H */

/* Define to 1 if you have the </usr/src/sys/i386/isa/sound/awe_voice.h>
   header file. */
/* #undef HAVE__USR_SRC_SYS_I386_ISA_SOUND_AWE_VOICE_H */

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
#define S_ISSOCK(mode) (1==0)
#define TIME_WITH_SYS_TIME 1
#define WORDS_BIGENDIAN 1

/* where rgb.txt is in */
#define X11_RGBFILE "/usr/X11R6/lib/X11/rgb.txt"

/* Defines the executable of xmllint */
#define XMLLINT "/sw/bin/xmllint"

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

#ifndef HAVE_S_ISSOCK
#define HAVE_S_ISSOCK
#define S_ISSOCK(mode) (1==0)
#endif



/*
 * On HP-UX, the declaration of vsnprintf() is needed every time !
 */

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
