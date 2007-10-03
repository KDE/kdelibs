project(kpty)

include_directories( ${KDE4_KDECORE_INCLUDES} )
include_directories( ${QT_INCLUDES} )

set(kpty_LIB_SRCS
   kpty.cpp
   kptydevice.cpp
   kptyprocess.cpp
)

kde4_add_library(kpty SHARED ${kpty_LIB_SRCS})

target_link_libraries(kpty ${KDE4_KDECORE_LIBS} ${UTIL_LIBRARY})

#set_target_properties(kdecore PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION} )
install(TARGETS kpty
             RUNTIME DESTINATION ${BIN_INSTALL_DIR}
             LIBRARY DESTINATION ${LIB_INSTALL_DIR}
             ARCHIVE DESTINATION ${LIB_INSTALL_DIR} )

install( FILES
   kpty_export.h
   kpty.h
   kptydevice.h
   kptyprocess.h
   DESTINATION ${INCLUDE_INSTALL_DIR}
)

########### next target ###############

kde4_add_executable(kgrantpty NOGUI kgrantpty.c)
target_link_libraries(kgrantpty  kdefakes )
install(TARGETS kgrantpty DESTINATION ${LIBEXEC_INSTALL_DIR} )

set( KDE4_KPTY_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} )
add_subdirectory( tests )
