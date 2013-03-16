project(kpty)

set(kpty_LIB_SRCS
   kpty.cpp
   kptydevice.cpp
   kptyprocess.cpp
)

kde4_add_library(kpty ${LIBRARY_TYPE} ${kpty_LIB_SRCS})
generate_export_header(kpty)

include(ConfigureChecks.cmake)
configure_file(config-pty.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-pty.h )

target_link_libraries(kpty ${KDE4_KDECORE_LIBS} ${UTIL_LIBRARY} ${UTEMPTER_LIBRARY})
target_link_libraries(kpty LINK_INTERFACE_LIBRARIES kdecore ki18n )

target_include_directories(kpty PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

set_target_properties(kpty PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS kpty EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kpty_export.h
   kpty.h
   kptydevice.h
   kptyprocess.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

########### next target ###############

if (NOT HAVE_OPENPTY)
   kde4_add_executable(kgrantpty NOGUI kgrantpty.c)
   target_link_libraries(kgrantpty kdefakes)
   install(TARGETS kgrantpty DESTINATION ${LIBEXEC_INSTALL_DIR})
endif (NOT HAVE_OPENPTY)

add_subdirectory( tests )
