
macro_bool_to_01(FAM_FOUND HAVE_FAM)
check_include_files(sys/inotify.h HAVE_SYS_INOTIFY_H)

configure_file(config-kdirwatch.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdirwatch.h )
