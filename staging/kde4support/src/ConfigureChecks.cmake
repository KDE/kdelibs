####### checks for kdecore/network (and netsupp.cpp) ###############
include(CheckIncludeFiles)
include(CheckFunctionExists)
# TODO: check whether CheckPrototypeExists.cmake is actually necessary or whether something else can be used. Alex
include(${CMAKE_SOURCE_DIR}/cmake/modules/CheckPrototypeExists.cmake)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckStructHasMember)
include(CheckCSourceRuns)
include(CMakePushCheckState)

set( KDELIBSUFF ${LIB_SUFFIX} )

# now check for dlfcn.h using the cmake supplied CHECK_INCLUDE_FILES() macro
# If definitions like -D_GNU_SOURCE are needed for these checks they
# should be added to _KDE4_PLATFORM_DEFINITIONS when it is originally
# defined outside this file.  Here we include these definitions in
# CMAKE_REQUIRED_DEFINITIONS so they will be included in the build of
# checks below.
set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})

cmake_push_check_state()
  set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${QT_INCLUDES}")
  if (QT_USE_FRAMEWORKS)
    set(CMAKE_REQUIRED_FLAGS "-F${QT_LIBRARY_DIR} ")
  endif (QT_USE_FRAMEWORKS)
  set (CMAKE_CXX_FLAGS_SAVED "${CMAKE_CXX_FLAGS}")

  # If Qt is built with reduce-relocations (The default) we need to add -fPIE here.
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")

  set(CMAKE_REQUIRED_LIBRARIES Qt5::Network)
  check_cxx_source_compiles(
"#include <QtNetwork/QSslSocket>
int main()
{
    QSslSocket *socket;
    return 0;
}" HAVE_QSSLSOCKET)

  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVED}")

  if (NOT HAVE_QSSLSOCKET)
     message(FATAL_ERROR "KDE Requires Qt to be built with SSL support")
  endif (NOT HAVE_QSSLSOCKET)
cmake_pop_check_state()

check_include_files("sys/types.h;sys/socket.h;net/if.h" HAVE_NET_IF_H)
check_include_files("sys/filio.h" HAVE_SYS_FILIO_H)
check_include_files(stropts.h HAVE_STROPTS_H)
check_include_files(paths.h       HAVE_PATHS_H)
check_include_files(sys/stat.h    HAVE_SYS_STAT_H) 
check_include_files(sys/time.h    HAVE_SYS_TIME_H) 
check_include_files(sys/filio.h  HAVE_SYS_FILIO_H)
check_include_file( "stropts.h" HAVE_STROPTS_H )
check_include_files(limits.h   HAVE_LIMITS_H) # for kcmdlineargs
check_include_files(unistd.h      HAVE_UNISTD_H)

# This is broken on OSX 10.6 (succeeds but shouldn't do) and doesn't exist
# on previous versions so don't do the check on APPLE.
if(NOT APPLE)
  check_function_exists(fdatasync      HAVE_FDATASYNC)  # kdecore, kate
endif(NOT APPLE)

check_function_exists(inet_pton        HAVE_INET_PTON)
check_function_exists(inet_ntop        HAVE_INET_NTOP)
check_function_exists(getprotobyname_r HAVE_GETPROTOBYNAME_R)
check_function_exists(poll             HAVE_POLL)
check_function_exists(getservbyname_r  HAVE_GETSERVBYNAME_R)
check_function_exists(getservbyport_r HAVE_GETSERVBYPORT_R)
check_function_exists(gethostbyname2   HAVE_GETHOSTBYNAME2)
check_function_exists(gethostbyname2_r HAVE_GETHOSTBYNAME2_R)
check_function_exists(gethostbyname    HAVE_GETHOSTBYNAME)
check_function_exists(gethostbyname_r  HAVE_GETHOSTBYNAME_R)
check_function_exists(if_nametoindex  HAVE_IF_NAMETOINDEX)

check_prototype_exists(getservbyname_r netdb.h      HAVE_GETSERVBYNAME_R_PROTO)

check_symbol_exists(posix_madvise   "sys/mman.h"               HAVE_MADVISE)

check_symbol_exists(freeaddrinfo    "sys/types.h;sys/socket.h;netdb.h"     HAVE_FREEADDRINFO)
check_symbol_exists(getnameinfo     "sys/types.h;sys/socket.h;netdb.h"     HAVE_GETNAMEINFO)
check_symbol_exists(getaddrinfo     "sys/types.h;sys/socket.h;netdb.h"     HAVE_GETADDRINFO)

check_symbol_exists(res_init        "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_RES_INIT)
# redundant? check_function_exists(res_init        HAVE_RES_INIT)

check_struct_has_member("struct sockaddr" sa_len "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_SA_LEN)

# check if gai_strerror exists even if EAI_ADDRFAMILY is not defined
set(CMAKE_EXTRA_INCLUDE_FILES "sys/socket.h;netdb.h")
check_prototype_exists(gai_strerror netdb.h HAVE_GAI_STRERROR_PROTO)

# check for existing datatypes
set(CMAKE_EXTRA_INCLUDE_FILES "sys/socket.h;netdb.h")
check_type_size("struct addrinfo" HAVE_STRUCT_ADDRINFO)
set(CMAKE_EXTRA_INCLUDE_FILES "sys/socket.h;netdb.h")
check_type_size("struct sockaddr_in6" HAVE_STRUCT_SOCKADDR_IN6)
set(CMAKE_EXTRA_INCLUDE_FILES)  #reset CMAKE_EXTRA_INCLUDE_FILES

check_struct_member("struct sockaddr" sa_len "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_SA_LEN)
check_struct_member(dirent d_type dirent.h HAVE_DIRENT_D_TYPE)
check_prototype_exists(res_init "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_RES_INIT_PROTO)

check_c_source_runs("
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netdb.h>
  #include <string.h>
  int main()
  {
    struct addrinfo hint, *res;
    int err;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_protocol = 0;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    err = getaddrinfo(0, \"18300\", &hint, &res);
    if (err != 0 || res == 0 || res->ai_family != AF_INET)
      return 1;
    return 0;
  }"
  HAVE_GOOD_GETADDRINFO
)

if( NOT HAVE_GOOD_GETADDRINFO )
  set( HAVE_BROKEN_GETADDRINFO 1 )
endif( NOT HAVE_GOOD_GETADDRINFO )
