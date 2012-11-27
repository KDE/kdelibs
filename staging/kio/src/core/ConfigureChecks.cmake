include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckStructHasMember)
include(CheckFunctionExists)
include(CheckSymbolExists)

check_function_exists(backtrace        HAVE_BACKTRACE)
check_struct_has_member("struct sockaddr" sa_len "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_SA_LEN)

find_package(ACL)
macro_bool_to_01(ACL_FOUND HAVE_LIBACL HAVE_POSIX_ACL)
macro_log_feature(ACL_FOUND "LibACL" "Support for manipulating access control lists" "ftp://oss.sgi.com/projects/xfs/cmd_tars" FALSE "" "STRONGLY RECOMMENDED")


