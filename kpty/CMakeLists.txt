project(kpty)

include_directories( ${KDE4_KDECORE_INCLUDES} )
include_directories( ${QT_INCLUDES} )

set(kpty_LIB_SRCS
   kpty.cpp
   kptydevice.cpp
   kptyprocess.cpp
)

kde4_add_library(kpty SHARED ${kpty_LIB_SRCS})

target_link_libraries(kpty ${KDE4_KDECORE_LIBS} ${UTIL_LIBRARY} ${UTEMPTER_LIBRARY})
target_link_libraries(kpty LINK_INTERFACE_LIBRARIES kdecore ${QT_QTCORE_LIBRARY} )

set_target_properties(kpty PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS kpty EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
   kpty_export.h
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
