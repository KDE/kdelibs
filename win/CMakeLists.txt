
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

add_definitions(-DMAKE_KDEWIN32_LIB -DUNICODE)

set(win_LIB_SRCS fcntl.c getenv.c kde_file_win.c net.c readdir.c
resource.c strndup.c time.c unistd.c fsync.c grp.c mmap.c pwd.c 
realpath.c signal.c syslog.c uname.c  win32_utils.c win32_utils2.cpp
bootstrap.cpp)

if(MINGW)
# do whatever required for mingw

endif(MINGW)

if(MSVC)
# do whatever required for msvc

endif(MSVC)

kde4_add_library(kdewin32 SHARED ${win_LIB_SRCS} )
target_link_libraries(kdewin32 ${QT_QTCORE_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} )
set_target_properties(kdecore PROPERTIES VERSION 4.2.0 SOVERSION 4 )

install_targets(/lib kdewin32 )