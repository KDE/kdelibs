include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckFunctionExists)

set( KDELIBSUFF ${LIB_SUFFIX} )

check_include_files(strings.h     HAVE_STRINGS_H)
check_include_files(sys/select.h  HAVE_SYS_SELECT_H)
check_include_files(sys/stat.h    HAVE_SYS_STAT_H)
check_include_files(paths.h       HAVE_PATHS_H) 

check_function_exists(initgroups HAVE_INITGROUPS)  
