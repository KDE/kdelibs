include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFile)
include(CheckIncludeFiles)

check_function_exists(backtrace   HAVE_BACKTRACE)
check_function_exists(setmntent   HAVE_SETMNTENT)
check_function_exists(getmntinfo  HAVE_GETMNTINFO)

check_library_exists(volmgt volmgt_running "" HAVE_VOLMGT)

check_include_files(sys/param.h   HAVE_SYS_PARAM_H)
check_include_files(sys/types.h   HAVE_SYS_TYPES_H)
check_include_files("sys/param.h;sys/mount.h"  HAVE_SYS_MOUNT_H)
check_include_files(sys/mntent.h  HAVE_SYS_MNTENT_H) 
check_include_files("stdio.h;sys/mnttab.h"  HAVE_SYS_MNTTAB_H)
check_include_files(fstab.h       HAVE_FSTAB_H)
check_include_files(mntent.h      HAVE_MNTENT_H)
check_include_files(sys/time.h    HAVE_SYS_TIME_H)
check_include_files(time.h        HAVE_TIME_H)

