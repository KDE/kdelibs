include(CheckTypeSize)
include(CheckStructMember)
include(CheckIncludeFile)
include(CheckIncludeFiles)

check_type_size(time_t SIZEOF_TIME_T)

check_include_files(sys/time.h    HAVE_SYS_TIME_H) 
check_include_files(time.h        HAVE_TIME_H)

check_struct_member(tm tm_zone time.h HAVE_STRUCT_TM_TM_ZONE)
check_struct_member(tm tm_gmtoff time.h HAVE_TM_GMTOFF)

