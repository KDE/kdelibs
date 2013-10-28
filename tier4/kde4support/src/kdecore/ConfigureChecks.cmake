include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckStructMember)
include(CheckIncludeFile)
include(CheckIncludeFiles)

check_function_exists(backtrace   HAVE_BACKTRACE)

check_include_files(sys/time.h    HAVE_SYS_TIME_H)
check_include_files(time.h        HAVE_TIME_H)

check_type_size(time_t SIZEOF_TIME_T)

check_struct_member(tm tm_zone time.h HAVE_STRUCT_TM_TM_ZONE)
check_struct_member(tm tm_gmtoff time.h HAVE_TM_GMTOFF)

## Should this be find_package(Gettext) instead, which seems more complete?
find_package(Libintl)
set_package_properties(LIBINTL PROPERTIES DESCRIPTION "Libintl" URL "http://www.gnu.org/software/gettext"
                       TYPE REQUIRED PURPOSE "Enables KDE to be available in many different languages")

