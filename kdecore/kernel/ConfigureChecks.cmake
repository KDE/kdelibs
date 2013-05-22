include(CheckIncludeFile)
include(CheckStructMember)

check_include_files(sys/stat.h    HAVE_SYS_STAT_H)
check_include_files(unistd.h      HAVE_UNISTD_H)

check_struct_member(dirent d_type dirent.h HAVE_DIRENT_D_TYPE)
