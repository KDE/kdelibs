include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckCXXSymbolExists)

check_include_files(sys/time.h    HAVE_SYS_TIME_H)

check_symbol_exists(strtoll         "stdlib.h"                 HAVE_STRTOLL)
