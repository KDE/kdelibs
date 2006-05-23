# NOTE: only add something here if it is really needed by all of kdelibs.
#     Otherwise please prefer adding to the relevant config-foo.h.cmake file,
#     and the CMakeLists.txt that generates it (or a separate ConfigureChecks.make file if you prefer)
#     to minimize recompilations and increase modularity.

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckPrototypeExists)
include(CheckTypeSize)
include(CheckStructMember)
include(CheckCXXSourceCompiles)

# The FindKDE4.cmake module sets _KDE4_PLATFORM_DEFINITIONS with
# definitions like _GNU_SOURCE that are needed on each platform.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})

#check for libz using the cmake supplied FindZLIB.cmake
macro_bool_to_01(ZLIB_FOUND HAVE_LIBZ)                  # zlib is required

macro_bool_to_01(BZIP2_FOUND HAVE_BZIP2_SUPPORT)        # kdecore  
if(BZIP2_FOUND AND BZIP2_NEED_PREFIX)
   set(NEED_BZ2_PREFIX 1)
endif(BZIP2_FOUND AND BZIP2_NEED_PREFIX)

macro_bool_to_01(CUPS_FOUND HAVE_CUPS)                  # kdeprint

macro_bool_to_01(CARBON_FOUND HAVE_CARBON)              # kdecore

macro_bool_to_01(GETTEXT_FOUND ENABLE_NLS)              # kdecore, khtml, kjs

# FIXME: Make this changeable!
# khtml svg support
set(SVG_SUPPORT 1)              # unused yet, but for the future

#now check for dlfcn.h using the cmake supplied CHECK_include_FILE() macro
# If definitions like -D_GNU_SOURCE are needed for these checks they
# should be added to _KDE4_PLATFORM_DEFINITIONS when it is originally
# defined outside this file.  Here we include these definitions in
# CMAKE_REQUIRED_DEFINITIONS so they will be included in the build of
# checks below.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})
if (WIN32)
   set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES} )
   set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES} )
endif (WIN32)

check_include_files(stdio.h       HAVE_STDIO_H)                        # various
check_include_files(stdlib.h      HAVE_STDLIB_H)                       # various
check_include_files(string.h      HAVE_STRING_H)                       # various
check_include_files(strings.h     HAVE_STRINGS_H)                      # various
check_include_files(ctype.h       HAVE_CTYPE_H)                        # libltdl
check_include_files(malloc.h      HAVE_MALLOC_H)                       # libltdl, khtml
check_include_files(memory.h      HAVE_MEMORY_H)                       # libltdl
check_include_files(dlfcn.h       HAVE_DLFCN_H)                        # various
check_include_files(sys/time.h    TIME_WITH_SYS_TIME)                  # kdecore, kioslave
check_include_files(float.h       HAVE_FLOAT_H)                        # kjs
check_include_files(crt_externs.h HAVE_CRT_EXTERNS_H)                  # kinit, config.h
check_include_files(dld.h         HAVE_DLD_H)                          # libltdl
check_include_files(dl.h          HAVE_DL_H)                           # libltdl

check_include_files(alloca.h      HAVE_ALLOCA_H)                       # kdecore, khtml
check_include_files(fstab.h       HAVE_FSTAB_H)                        # kio, kdecore
check_include_files(ieeefp.h      HAVE_IEEEFP_H)                       # kjs
check_include_files(limits.h      HAVE_LIMITS_H)                       # various
check_include_files("sys/types.h;libutil.h" HAVE_LIBUTIL_H)            # kdesu, kdecore
check_include_files(util.h        HAVE_UTIL_H)                         # kdesu, kdecore
check_include_files(mntent.h      HAVE_MNTENT_H)                       # kio, kdecore
check_include_files("pthread.h;pthread_np.h" HAVE_PTHREAD_NP_H)        # kjs
check_include_files(pty.h         HAVE_PTY_H)                          # kdecore
check_include_files(sysent.h      HAVE_SYSENT_H)                       # kdecore
check_include_files(sys/bitypes.h HAVE_SYS_BITYPES_H)                  # kwallet
check_include_files("sys/types.h;sys/mman.h" HAVE_SYS_MMAN_H)          # kdecore
check_include_files(sys/stat.h    HAVE_SYS_STAT_H)                     # various
check_include_files(sys/ucred.h   HAVE_SYS_UCRED_H)                    # kio
check_include_files(sys/types.h   HAVE_SYS_TYPES_H)                    # various
check_include_files(sys/select.h  HAVE_SYS_SELECT_H)                   # various
check_include_files(sys/stropts.h HAVE_SYS_STROPTS_H)                  # kdecore
check_include_files(sys/param.h   HAVE_SYS_PARAM_H)                    # various
check_include_files(sys/mnttab.h  HAVE_SYS_MNTTAB_H)                   # kio, kdecore
check_include_files(sys/mntent.h  HAVE_SYS_MNTENT_H)                   # kio, kdecore
check_include_files("sys/param.h;sys/mount.h"  HAVE_SYS_MOUNT_H)       # kio, kdecore
check_include_files(termios.h     HAVE_TERMIOS_H)                      # kdecore 
check_include_files(termio.h      HAVE_TERMIO_H)                       # kdecore
check_include_files(unistd.h      HAVE_UNISTD_H)                       # various
check_include_files(stdint.h      HAVE_STDINT_H)                       # various  
check_include_files("sys/types.h;netinet/in.h"  HAVE_NETINET_IN_H)     # kio
check_include_files(paths.h       HAVE_PATHS_H)                        # kdecore, kio  

check_include_files(errno.h       HAVE_ERRNO_H)                        # kjs, errno.h is used in many places, but only guarded in kjs/
check_include_files(sys/time.h    HAVE_SYS_TIME_H)                     # various
check_include_files(sys/timeb.h   HAVE_SYS_TIMEB_H)                    # kjs

check_include_files(arpa/nameser8_compat.h HAVE_ARPA_NAMESER8_COMPAT_H) # kio

if (X11_XTest_FOUND)                                                   # kdecore
  set(HAVE_XTEST 1)
else (X11_XTest_FOUND)
  set(HAVE_XTEST 0)
endif (X11_XTest_FOUND)


# Use check_symbol_exists to check for symbols in a reliable
# cross-platform manner.  It accounts for different calling
# conventions and the possibility that the symbol is defined as a
# macro.  Note that some symbols require multiple includes in a
# specific order.  Refer to the man page for each symbol for which a
# check is to be added to get the proper set of headers.
check_symbol_exists(strncasecmp     "strings.h"                HAVE_STRNCASECMP) # kjs
check_symbol_exists(strchr          "string.h"                 HAVE_STRCHR)      # libltdl (kdecore ?)
check_symbol_exists(strcmp          "string.h"                 HAVE_STRCMP)      # libltdl
check_symbol_exists(strrchr         "string.h"                 HAVE_STRRCHR)     # libltdl
check_symbol_exists(strtoll         "stdlib.h"                 HAVE_STRTOLL)     # kioslave
check_symbol_exists(snprintf        "stdio.h"                  HAVE_SNPRINTF)    # kjs
check_symbol_exists(S_ISSOCK        "sys/stat.h"               HAVE_S_ISSOCK)    # config.h
check_symbol_exists(vsnprintf       "stdio.h"                  HAVE_VSNPRINTF)   # config.h


check_function_exists(posix_fadvise    HAVE_FADVISE)                  # kioslave
check_function_exists(index            HAVE_INDEX)                    # libltdl
check_function_exists(rindex           HAVE_RINDEX)                   # libltdl
check_function_exists(memcpy           HAVE_MEMCPY)                   # libltdl, kdecore/malloc
check_function_exists(bcopy            HAVE_BCOPY)                    # libltdl
check_function_exists(backtrace        HAVE_BACKTRACE)                # kdecore, kio
check_function_exists(getpagesize      HAVE_GETPAGESIZE)              # kdecore/malloc, khtml
check_function_exists(getpeereid       HAVE_GETPEEREID)               # kdesu
check_function_exists(getpt           HAVE_GETPT)                     # kdesu
check_function_exists(grantpt         HAVE_GRANTPT)                   # kdecore, kdesu
check_function_exists(madvise         HAVE_MADVISE)                   # kdecore
check_function_exists(mmap            HAVE_MMAP)                      # kdecore, kdecore/malloc, khtml
check_function_exists(readdir_r       HAVE_READDIR_R)                 # kio
check_function_exists(sendfile        HAVE_SENDFILE)                  # kioslave
check_function_exists(setlocale       HAVE_SETPRIORITY)               # kdesu
check_function_exists(srandom         HAVE_SRANDOM)                   # config.h
check_function_exists(unlockpt        HAVE_UNLOCKPT)                  # kdecore, kdesu
check_function_exists(_NSGetEnviron   HAVE_NSGETENVIRON)              # kinit, config.h


check_library_exists(utempter addToUtmp "" HAVE_UTEMPTER)             # kdecore (kpty)
check_library_exists(volmgt volmgt_running "" HAVE_VOLMGT)            # various

# Check for libresolv
# e.g. on slackware 9.1 res_init() is only a define for __res_init, so we check both, Alex
set(HAVE_RESOLV_LIBRARY FALSE)                                        # kdecore, kdecore/network, kpac
check_library_exists(resolv res_init "" HAVE_RES_INIT_IN_RESOLV_LIBRARY)
check_library_exists(resolv __res_init "" HAVE___RES_INIT_IN_RESOLV_LIBRARY)
if (HAVE___RES_INIT_IN_RESOLV_LIBRARY OR HAVE_RES_INIT_IN_RESOLV_LIBRARY)
   set(HAVE_RESOLV_LIBRARY TRUE)
endif (HAVE___RES_INIT_IN_RESOLV_LIBRARY OR HAVE_RES_INIT_IN_RESOLV_LIBRARY)

check_library_exists(util  openpty "" HAVE_OPENPTY)
if (HAVE_OPENPTY)
  set(UTIL_LIBRARY util)
endif (HAVE_OPENPTY)


# it seems this isn't used anywhere
#find_library(ICE_LIB NAMES ICE PATHS /usr/X11/lib)
#check_library_exists(${ICE_LIB} _IceTransNoListen "" HAVE__ICETRANSNOLISTEN)

#set(CMAKE_REQUIRED_LIBRARIES crypt)
#check_function_exists(crypt "" HAVE_CRYPT)
#set(CMAKE_REQUIRED_LIBRARIES util)
#check_function_exists(openpty "" HAVE_OPENPTY)
#set(CMAKE_REQUIRED_LIBRARIES)

#dlopen stuff
set(LIBDL)
set(HAVE_LIBDL)                                         # kdecore
# on FreeBSD dlopen is in libc, on Linux it's in libdl
check_library_exists(dl dlopen ""  DLOPEN_IN_LIBDL)
check_function_exists(dlopen       DLOPEN_IN_LIBC)

if (DLOPEN_IN_LIBC)
   set(LIBDL c)
   set(HAVE_LIBDL TRUE)
   check_function_exists(dlerror HAVE_DLERROR)
endif (DLOPEN_IN_LIBC)

if (DLOPEN_IN_LIBDL)
   set(LIBDL dl)
   set(HAVE_LIBDL TRUE)
   check_library_exists(dl dlerror ""  HAVE_DLERROR)     # libltdl
endif (DLOPEN_IN_LIBDL)

check_function_exists(shl_load   HAVE_SHL_LOAD)          # libltdl
check_function_exists(dld_init   HAVE_DLD)               # libltdl

check_function_exists(getmntinfo HAVE_GETMNTINFO)        # kdecore, kio
check_function_exists(initgroups HAVE_INITGROUPS)        # kdecore, kdesu
check_function_exists(mkstemps   HAVE_MKSTEMPS)          # dcop, kdecore/fakes.c
check_function_exists(mkstemp    HAVE_MKSTEMP)           # kdecore/fakes.c
check_function_exists(mkdtemp    HAVE_MKDTEMP)           # kdecore/fakes.c
check_function_exists(ptsname    HAVE_PTSNAME)           # kdecore, kdesu
check_function_exists(random     HAVE_RANDOM)            # kdecore/fakes.c
check_function_exists(revoke     HAVE_REVOKE)            # kdecore/fakes.c
check_function_exists(strlcpy    HAVE_STRLCPY)           # kdecore/fakes.c
check_function_exists(strlcat    HAVE_STRLCAT)           # kdecore/fakes.c
check_function_exists(setenv     HAVE_SETENV)            # kdecore/fakes.c
check_function_exists(seteuid    HAVE_SETEUID)           # kdecore/fakes.c
check_function_exists(setmntent  HAVE_SETMNTENT)         # kio, kdecore
check_function_exists(unsetenv   HAVE_UNSETENV)          # kdecore/fakes.c
check_function_exists(usleep     HAVE_USLEEP)            # kdecore/fakes.c, kdeui/qxembed
check_function_exists(_getpty    HAVE__GETPTY)           # kdesu

check_function_exists(_finite    HAVE_FUNC__FINITE)      # kjs
check_function_exists(finite     HAVE_FUNC_FINITE)       # kjs
check_function_exists(isinf      HAVE_FUNC_ISINF)        # kjs
check_function_exists(isnan      HAVE_FUNC_ISNAN)        # kjs

# check for prototypes [for functions provided by kdefakes when not available]

check_prototype_exists(mkstemps "stdlib.h;unistd.h" HAVE_MKSTEMPS_PROTO)
check_prototype_exists(mkdtemp "stdlib.h;unistd.h"  HAVE_MKDTEMP_PROTO)
check_prototype_exists(mkstemp "stdlib.h;unistd.h"  HAVE_MKSTEMP_PROTO)
check_prototype_exists(strlcat string.h             HAVE_STRLCAT_PROTO)
check_prototype_exists(strlcpy string.h             HAVE_STRLCPY_PROTO)
check_prototype_exists(random stdlib.h              HAVE_RANDOM_PROTO)
check_prototype_exists(res_init "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_RES_INIT_PROTO)
check_prototype_exists(setenv stdlib.h              HAVE_SETENV_PROTO)
check_prototype_exists(srandom stdlib.h             HAVE_SRANDOM_PROTO)
check_prototype_exists(unsetenv stdlib.h            HAVE_UNSETENV_PROTO)
check_prototype_exists(usleep unistd.h              HAVE_USLEEP_PROTO)
check_prototype_exists(initgroups "unistd.h;sys/types.h;unistd.h;grp.h" HAVE_INITGROUPS_PROTO)
check_prototype_exists(setreuid unistd.h            HAVE_SETREUID_PROTO)
check_prototype_exists(seteuid unistd.h             HAVE_SETEUID_PROTO)

# check for existing datatypes

set(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
check_type_size("struct ucred" HAVE_STRUCT_UCRED)                # kdesu

set(CMAKE_EXTRA_INCLUDE_FILES)  #reset CMAKE_EXTRA_INCLUDE_FILES

set(CONFIG_QT_DOCDIR "\"${QT_DOC_DIR}/html/\"")
set(CONFIG_KDELIBS_DOXYDIR "\"/tmp/\"")

check_cxx_source_compiles("
  #include <sys/types.h>
  #include <sys/statvfs.h>
  int main(){
    struct statvfs *mntbufp;
    int flags;
    return getmntinfo(&mntbufp, flags);
  }
" GETMNTINFO_USES_STATVFS )

check_struct_member(tm tm_zone time.h HAVE_STRUCT_TM_TM_ZONE)  # kdecore
check_struct_member(tm tm_gmtoff time.h HAVE_TM_GMTOFF)        # kdecore
