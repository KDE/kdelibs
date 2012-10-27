include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckStructHasMember)
include(CheckFunctionExists)
include(CheckSymbolExists)

check_function_exists(backtrace        HAVE_BACKTRACE)
check_struct_has_member("struct sockaddr" sa_len "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_SA_LEN)
