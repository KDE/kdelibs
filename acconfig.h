/* Define if the C++ compiler supports BOOL */
#undef HAVE_BOOL

#undef VERSION

#undef PACKAGE

/* defines if having libgif (always 1) */
#undef HAVE_LIBGIF

/* defines if having libjpeg (always 1) */
#undef HAVE_LIBJPEG

/* defines if having libtiff */
#undef HAVE_LIBTIFF

/* defines if having libpng */
#undef HAVE_LIBPNG

/* defines which to take for ksize_t */
#undef ksize_t

/* define if you have setenv */
#undef HAVE_FUNC_SETENV

/* Define to 1 if NLS is requested.  */
#undef ENABLE_NLS

/* Define as 1 if you have the stpcpy function.  */
#undef HAVE_STPCPY

/* Define if your locale.h file contains LC_MESSAGES.  */
#undef HAVE_LC_MESSAGES    

/* Define if you need the GNU extensions to compile */
#undef _GNU_SOURCE

/* Define if you have getdomainname */
#undef HAVE_GETDOMAINNAME

/* Define if you have gethostname */
#undef HAVE_GETHOSTNAME

/* Define if you have usleep */
#undef HAVE_USLEEP

/* Define if you have random */
#undef HAVE_RANDOM

/* This is the prefix of the below paths. This may change in the future */
#undef KDEDIR

/* Where your docs should go to. */
#undef KDE_HTMLDIR

/* Where your application file (.kdelnk) should go to. */
#undef KDE_APPSDIR

/* Where your icon should go to. */
#undef KDE_ICONDIR

 /* Where your sound data should go to. */
#undef KDE_SOUNDDIR

/* Where you install application data. */
#undef KDE_DATADIR

/* Where translation files should go to. */
#undef KDE_LOCALE

/* Where cgi-bin executables should go to. */
#undef KDE_CGIDIR

/* Where mimetypes should go to. */
#undef KDE_MIMEDIR

/* Where mimetypes should go to. */
#undef KDE_TOOLBARDIR

/* Where general wallpapers should go to. */
#undef KDE_WALLPAPERDIR

/* Where binaries should be installed. */
#undef KDE_BINDIR

/* Where KDE parts should be installed. */
#undef KDE_PARTSDIR

/* Where KDE config files should be installed */
#undef KDE_CONFIGDIR

/* If the platform supports strdup */
#undef HAVE_STRDUP

/* Define if you have an SGI like STL implementation */
#undef HAVE_SGI_STL

/* Define if you have an HP like STL implementation */
#undef HAVE_HP_STL


#ifndef HAVE_BOOL
#define HAVE_BOOL
typedef int bool;
#ifdef __cplusplus
const bool false = 0;
const bool true = 1;
#else
#define false (bool)0;
#define true (bool)1;
#endif
#endif

/* this is needed for Solaris and others */
#ifndef HAVE_GETDOMAINNAME
#define HAVE_GETDOMAINNAME
#ifdef __cplusplus
extern "C"
#endif
int getdomainname (char *Name, int Namelen);
#endif  

#ifndef HAVE_GETHOSTNAME
#define HAVE_GETHOSTNAME
#ifdef __cplusplus  
extern "C" 
#endif
int gethostname (char *Name, int Namelen);
#endif  

/*
 * This is needed for systems with broken headers that won't work with
 *  gcc -ansi
 */
#ifndef HAVE_STRDUP
#define kstrdup qstrdup
#else
#define kstrdup strdup
#endif

/*
 * This is an ugly little hack incase someone uses the inline keyword,
 * in a straight C file with AIX's xlc.
 */
#ifndef __AIX32
#ifndef __cplusplus
#define inline __inline
#endif
#endif

/*
 * jpeg.h needs HAVE_BOOLEAN, when the system uses boolean in system
 * headers and I'm too lazy to write a configure test as long as only
 * unixware is related
 */
#ifdef _UNIXWARE
#define HAVE_BOOLEAN
#endif

