include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckPrototypeExists)

check_include_files(arpa/nameser_compat.h HAVE_ARPA_NAMESER_COMPAT_H)
check_include_files(arpa/nameser8_compat.h HAVE_ARPA_NAMESER8_COMPAT_H)
check_include_files("sys/types.h;netinet/in.h"  HAVE_NETINET_IN_H) 
check_prototype_exists(res_init "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_RES_INIT_PROTO)

# Check for libresolv
# e.g. on slackware 9.1 res_init() is only a define for __res_init, so we check both, Alex
set(HAVE_RESOLV_LIBRARY FALSE)                                        # kdecore, kdecore/network, kpac
check_library_exists(resolv res_init "" HAVE_RES_INIT_IN_RESOLV_LIBRARY)
check_library_exists(resolv __res_init "" HAVE___RES_INIT_IN_RESOLV_LIBRARY)
if (HAVE___RES_INIT_IN_RESOLV_LIBRARY OR HAVE_RES_INIT_IN_RESOLV_LIBRARY)
	set(HAVE_RESOLV_LIBRARY TRUE)
endif (HAVE___RES_INIT_IN_RESOLV_LIBRARY OR HAVE_RES_INIT_IN_RESOLV_LIBRARY)
