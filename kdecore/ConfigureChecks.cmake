include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckCXXSymbolExists)
include(CheckStructMember)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckPrototypeExists)

check_include_files(sys/time.h    HAVE_SYS_TIME_H)
check_include_files(time.h        HAVE_TIME_H)
check_include_files(alloca.h      HAVE_ALLOCA_H)
check_include_files(string.h      HAVE_STRING_H)
check_include_files(strings.h     HAVE_STRINGS_H)
check_include_files(unistd.h      HAVE_UNISTD_H)
check_include_files(stdlib.h      HAVE_STDLIB_H)
check_include_files(sys/stat.h    HAVE_SYS_STAT_H)
check_include_files(sys/types.h   HAVE_SYS_TYPES_H)
check_include_files(sys/select.h  HAVE_SYS_SELECT_H)
check_include_files(errno.h       HAVE_ERRNO_H)
check_include_files("stdio.h;sys/mnttab.h"  HAVE_SYS_MNTTAB_H)
check_include_files(mntent.h      HAVE_MNTENT_H)
check_include_files(sys/mntent.h  HAVE_SYS_MNTENT_H)
check_include_files("sys/param.h;sys/mount.h"  HAVE_SYS_MOUNT_H)
check_include_files(sys/param.h   HAVE_SYS_PARAM_H)
check_include_files(fstab.h       HAVE_FSTAB_H)
check_include_files(sys/time.h    HAVE_SYS_TIME_H)
check_include_files(sys/time.h    TIME_WITH_SYS_TIME)
check_include_files(time.h        HAVE_TIME_H)

check_function_exists(usleep     HAVE_USLEEP)
check_function_exists(setmntent   HAVE_SETMNTENT)
check_function_exists(getmntinfo  HAVE_GETMNTINFO)
check_function_exists(mkstemps    HAVE_MKSTEMPS)
check_function_exists(mkstemp     HAVE_MKSTEMP)
check_function_exists(mkstemps    HAVE_MKSTEMPS)
check_function_exists(random      HAVE_RANDOM)
check_function_exists(seteuid     HAVE_SETEUID)

check_library_exists(volmgt volmgt_running "" HAVE_VOLMGT)

check_struct_member(dirent d_type dirent.h HAVE_DIRENT_D_TYPE)

check_cxx_source_compiles("
  #include <sys/types.h>
  #include <sys/statvfs.h>
  int main(){
  	struct statvfs *mntbufp;
	int flags;
	return getmntinfo(&mntbufp, flags);
	}
" GETMNTINFO_USES_STATVFS )

check_prototype_exists(strlcat string.h             HAVE_STRLCAT_PROTO)
check_prototype_exists(strlcpy string.h             HAVE_STRLCPY_PROTO)
check_prototype_exists(random stdlib.h              HAVE_RANDOM_PROTO)
check_prototype_exists(setenv stdlib.h              HAVE_SETENV_PROTO)
check_prototype_exists(usleep unistd.h              HAVE_USLEEP_PROTO)
check_prototype_exists(initgroups "unistd.h;sys/types.h;unistd.h;grp.h" HAVE_INITGROUPS_PROTO)
check_prototype_exists(mkstemps "stdlib.h;unistd.h" HAVE_MKSTEMPS_PROTO)
check_prototype_exists(mkstemp "stdlib.h;unistd.h"  HAVE_MKSTEMP_PROTO)
check_symbol_exists(getgrouplist    "unistd.h;grp.h"           HAVE_GETGROUPLIST)
