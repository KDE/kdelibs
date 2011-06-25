include(CheckIncludeFiles)
check_include_files(arpa/nameser_compat.h HAVE_ARPA_NAMESER_COMPAT_H)
check_include_files(arpa/nameser8_compat.h HAVE_ARPA_NAMESER8_COMPAT_H)
check_include_files("sys/types.h;netinet/in.h"  HAVE_NETINET_IN_H) 
