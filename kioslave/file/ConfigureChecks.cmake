include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFile)
include(CheckIncludeFiles)

check_include_files(sys/time.h    HAVE_SYS_TIME_H)
check_include_files(string.h      HAVE_STRING_H)
check_include_files(limits.h      HAVE_LIMITS_H)

check_function_exists(posix_fadvise    HAVE_FADVISE)                  # kioslave

check_library_exists(volmgt volmgt_running "" HAVE_VOLMGT)
