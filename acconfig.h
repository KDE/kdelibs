
#undef VERSION

#undef PACKAGE

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

#undef HAVE_BOOL

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

#undef HAVE_GETDOMAINNAME

/* this is needed for Solaris and others */
#ifndef HAVE_GETDOMAINNAME
#define HAVE_GETDOMAINNAME
#ifdef __cplusplus
extern "C"
#endif
int getdomainname (char *Name, int Namelen);
#endif  

#undef HAVE_GETHOSTNAME

#ifndef HAVE_GETHOSTNAME
#define HAVE_GETHOSTNAME
#ifdef __cplusplus  
extern "C" 
#endif
int gethostname (char *Name, int Namelen);
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

#undef HAVE_RANDOM

/* random() returns a value between 0 and RANDOM_MAX.
 * RANDOM_MAX is needed to generate good random numbers. (Nicolas)
 */
#ifndef HAVE_RANDOM
#define HAVE_RANDOM
#define RANDOM_MAX 2^31 // random() is lrand48()
long int random(void); // defined in fakes.cpp
void srandom(unsigned int seed);
#else
#define RANDOM_MAX RAND_MAX // normal random()
#endif

#undef HAVE_S_ISSOCK

#ifndef HAVE_S_ISSOCK
#define HAVE_S_ISSOCK
#define S_ISSOCK(mode) (1==0)
#endif


