set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_SOURCE_DIR}/cmake/modules) #TODO: remove cmake/modules when extra-cmake-modules is done

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckStructMember)
include(CheckFunctionExists)
include(CheckSymbolExists)

set( KDELIBSUFF ${LIB_SUFFIX} )

check_include_files(sys/stat.h    HAVE_SYS_STAT_H)
check_function_exists(backtrace        HAVE_BACKTRACE)
check_struct_member("struct sockaddr" sa_len "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_SA_LEN)
