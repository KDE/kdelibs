include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFile)
include(CheckIncludeFiles)

check_function_exists(backtrace   HAVE_BACKTRACE)

check_include_files(sys/time.h    HAVE_SYS_TIME_H)
check_include_files(time.h        HAVE_TIME_H)

