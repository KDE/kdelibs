project(kdewin32)

# not especially nice, but seems to work
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake/modules")

if (NOT WIN32)
   message(FATAL_ERROR "This directory can only be built under Windows")
endif (NOT WIN32)

find_package(Qt4 REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include )
include_directories(${QT_INCLUDES})

# find the headers
file(GLOB headers include/*.h)

if(MINGW)
   # we are using mingw, so do whatever required for mingw
   include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include/mingw )
   # find the headers for mingw
   file(GLOB_RECURSE _tmp_headers include/mingw/*.h)
else (MINGW)
   # we are using msvc, so do whatever required for msvc
   include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include/msvc )
   # is this also required for mingw ?
   add_definitions(-D_WINSOCKAPI_)
   # find the headers for msvc
   file(GLOB_RECURSE _tmp_headers include/msvc/*.h)
endif(MINGW)
set(headers ${headers} ${_tmp_headers})


add_definitions(-DMAKE_KDEWIN32_LIB -DUNICODE )

set(win_LIB_SRCS fcntl.c getenv.c kde_file_win.c net.c readdir.c
resource.c strndup.c time.c unistd.c fsync.c grp.c mmap.c pwd.c 
realpath.c signal.c syslog.c uname.c  win32_utils.c win32_utils2.cpp
bootstrap.cpp)


add_library(kdewin32 SHARED ${win_LIB_SRCS} )
target_link_libraries(kdewin32 ${QT_QTCORE_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ws2_32 )
set_target_properties(kdecore PROPERTIES VERSION 4.2.0 SOVERSION 4 )

install_targets(/lib kdewin32 )

install_files(/include FILES ${headers} )
