project(kdewin32)

if (NOT WIN32)
   message(FATAL_ERROR "This directory can only be built under Windows")
endif (NOT WIN32)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake/modules")

find_package(Qt4 REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )
include_directories(${QT_INCLUDES})

# find the headers
file(GLOB _win_headers include/*.h)

if(MINGW)
   # we are using mingw, so do whatever required for mingw
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/mingw )
   # find the headers for mingw
   file(GLOB _arpa_headers include/mingw/arpa/*.h)
   file(GLOB _asm_headers include/mingw/asm/*.h)
   file(GLOB _net_headers include/mingw/net/*.h)
   file(GLOB _netinet_headers include/mingw/netinet/*.h)
   file(GLOB _sys_headers include/mingw/sys/*.h)
   file(GLOB _headers include/mingw/*.h)
   set(_incDirPrefix mingw)
else (MINGW)
   # we are using msvc, so do whatever required for msvc
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/msvc )
   # is this also required for mingw ?
   add_definitions(-DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_DEPRECATE)
   # find the headers for msvc
   file(GLOB _arpa_headers include/msvc/arpa/*.h)
   file(GLOB _asm_headers include/msvc/asm/*.h)
   file(GLOB _machine_headers include/msvc/machine/*.h)
   file(GLOB _netinet_headers include/msvc/netinet/*.h)
   file(GLOB _sys_headers include/msvc/sys/*.h)
   file(GLOB _headers include/msvc/*.h)

   set(_incDirPrefix msvc)

endif(MINGW)

add_definitions(-DUNICODE -DWIN32_LEAN_AND_MEAN )

add_subdirectory(iceauth)

set(win_LIB_SRCS
   src/bootstrap.cpp
   src/dummy.cpp
   src/fcntl.c
   src/fsync.c
   src/getenv.c
   src/grp.c
   src/kde_file_win.c
   src/mmap.c
   src/net.c
   src/pwd.c
   src/readdir.c
   src/realpath.c
   src/resource.c
   src/signal.c
   src/stdlib.c
   src/string.c
   src/syslog.c
   src/time.c
   src/uname.c
   src/unistd.c
   src/win32_utils.c
   src/win32_utils2.cpp
   )


add_library(kdewin32 SHARED ${win_LIB_SRCS} )
target_link_libraries(kdewin32 ${QT_QTCORE_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} advapi32 shell32 ws2_32 )
set_target_properties(kdewin32 PROPERTIES DEFINE_SYMBOL MAKE_KDEWIN32_LIB )
set_target_properties(kdecore PROPERTIES VERSION 4.2.0 SOVERSION 4 )

install_targets(/lib kdewin32 )

install_files(/include/${_incDirPrefix}/arpa FILES ${_arpa_headers} )
install_files(/include/${_incDirPrefix}/asm FILES ${_asm_headers} )
install_files(/include/${_incDirPrefix}/machine FILES ${_machine_headers} )
install_files(/include/${_incDirPrefix}/netinet FILES ${_netinet_headers} )
install_files(/include/${_incDirPrefix}/net FILES ${_net_headers} )
install_files(/include/${_incDirPrefix}/sys FILES ${_sys_headers} )
install_files(/include/${_incDirPrefix} FILES ${_headers} )
install_files(/include FILES ${_win_headers} )
