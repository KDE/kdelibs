
project(sonnet)

include_directories( ${KDE4_KPARTS_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/kutils )
include_directories( ${CMAKE_SOURCE_DIR}/sonnet/spelling )
include_directories( ${CMAKE_SOURCE_DIR}/sonnet/misc )

set(sonnetbase_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/sonnet/spelling/loader.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/spelling/client.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/misc/filter.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/misc/settings.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/misc/backgroundchecker.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/misc/backgroundthread.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/misc/backgroundengine.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/misc/defaultdictionary.cpp
)

# for the subdirs
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( ui )
add_subdirectory( tests )
add_subdirectory( plugins )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES spelling/sonnetspeller.desktop )
install_files( /include/sonnet FILES settings.h filter.h   dictionary.h loader.h   backgroundchecker.h defaultdictionary.h  )




