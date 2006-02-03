project(kdewin32)

if (NOT WIN32)
   message(FATAL_ERROR "This directory can only be built under Windows")
endif (NOT WIN32)

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
   add_definitions(-D_WINSOCKAPI_ -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_DEPRECATE)
   # find the headers for msvc
   file(GLOB _arpa_headers include/msvc/arpa/*.h)
   file(GLOB _asm_headers include/msvc/asm/*.h)
   file(GLOB _machine_headers include/msvc/machine/*.h)
   file(GLOB _netinet_headers include/msvc/netinet/*.h)
   file(GLOB _sys_headers include/msvc/sys/*.h)
   file(GLOB _headers include/msvc/*.h)

   set(_incDirPrefix msvc)

endif(MINGW)

add_definitions(-DUNICODE )

set(win_LIB_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/src/bootstrap.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/dummy.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/fcntl.c
${CMAKE_CURRENT_SOURCE_DIR}/src/fsync.c
${CMAKE_CURRENT_SOURCE_DIR}/src/getenv.c
${CMAKE_CURRENT_SOURCE_DIR}/src/grp.c
${CMAKE_CURRENT_SOURCE_DIR}/src/kde_file_win.c
${CMAKE_CURRENT_SOURCE_DIR}/src/mmap.c
${CMAKE_CURRENT_SOURCE_DIR}/src/net.c
${CMAKE_CURRENT_SOURCE_DIR}/src/pwd.c
${CMAKE_CURRENT_SOURCE_DIR}/src/readdir.c
${CMAKE_CURRENT_SOURCE_DIR}/src/realpath.c
${CMAKE_CURRENT_SOURCE_DIR}/src/resource.c
${CMAKE_CURRENT_SOURCE_DIR}/src/signal.c
${CMAKE_CURRENT_SOURCE_DIR}/src/string.c
${CMAKE_CURRENT_SOURCE_DIR}/src/syslog.c
${CMAKE_CURRENT_SOURCE_DIR}/src/time.c
${CMAKE_CURRENT_SOURCE_DIR}/src/uname.c
${CMAKE_CURRENT_SOURCE_DIR}/src/unistd.c
${CMAKE_CURRENT_SOURCE_DIR}/src/win32_utils.c
${CMAKE_CURRENT_SOURCE_DIR}/src/win32_utils2.cpp
)


kde4_add_library(kdewin32 SHARED ${win_LIB_SRCS} )
target_link_libraries(kdewin32 ${QT_QTCORE_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ws2_32 )
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




