####### checks for kdecore/network (and netsupp.cpp) ###############
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckPrototypeExists)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckStructMember)

macro_push_required_vars()
  set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${QT_INCLUDE_DIR}")
  if (QT_USE_FRAMEWORKS)
    set(CMAKE_REQUIRED_FLAGS "-F${QT_LIBRARY_DIR} ")
  endif (QT_USE_FRAMEWORKS)
  check_cxx_source_compiles(
"#include <QtNetwork/QSslSocket>
int main()
{
    QSslSocket *socket;
    return 0;
}" HAVE_QSSLSOCKET)
  if (NOT HAVE_QSSLSOCKET)
    message(SEND_ERROR "KDE Requires Qt to be built with SSL support")
  endif (NOT HAVE_QSSLSOCKET)
macro_pop_required_vars()

check_include_files("sys/types.h;sys/socket.h;net/if.h" HAVE_NET_IF_H)
check_include_files("sys/filio.h" HAVE_SYS_FILIO_H)
check_include_files(stropts.h HAVE_STROPTS_H)

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

check_symbol_exists(freeaddrinfo    "sys/types.h;sys/socket.h;netdb.h"     HAVE_FREEADDRINFO)
check_symbol_exists(getnameinfo     "sys/types.h;sys/socket.h;netdb.h"     HAVE_GETNAMEINFO)
check_symbol_exists(getaddrinfo     "sys/types.h;sys/socket.h;netdb.h"     HAVE_GETADDRINFO)

check_symbol_exists(res_init        "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_RES_INIT)
# redundant? check_function_exists(res_init        HAVE_RES_INIT)

check_struct_member("struct sockaddr" sa_len "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_SA_LEN)

# check if gai_strerror exists even if EAI_ADDRFAMILY is not defined
set(CMAKE_EXTRA_INCLUDE_FILES "sys/socket.h;netdb.h")
check_prototype_exists(gai_strerror netdb.h HAVE_GAI_STRERROR_PROTO)

# check for existing datatypes
set(CMAKE_EXTRA_INCLUDE_FILES "sys/socket.h;netdb.h")
check_type_size("struct addrinfo" HAVE_STRUCT_ADDRINFO)
set(CMAKE_EXTRA_INCLUDE_FILES "sys/socket.h;netdb.h")
check_type_size("struct sockaddr_in6" HAVE_STRUCT_SOCKADDR_IN6)
set(CMAKE_EXTRA_INCLUDE_FILES)  #reset CMAKE_EXTRA_INCLUDE_FILES
