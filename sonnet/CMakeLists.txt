
project(sonnet)

add_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)

include_directories( ${KDE4_KDEUI_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/sonnet/spelling )
include_directories( ${CMAKE_SOURCE_DIR}/sonnet/misc )

set(sonnetbase_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/sonnet/spelling/loader.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/spelling/client.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/spelling/speller.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/misc/filter.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/misc/settings.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/misc/backgroundchecker.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/misc/backgroundthread.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/misc/backgroundengine.cpp
  ${CMAKE_SOURCE_DIR}/sonnet/misc/defaultdictionary.cpp
)

kde4_automoc(${sonnetbase_STAT_SRCS})

kde4_add_library(sonnetcore SHARED ${sonnetbase_STAT_SRCS})

target_link_libraries(sonnetcore ${KDE4_KDECORE_LIBS} )

set_target_properties(sonnetcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS sonnetcore DESTINATION ${LIB_INSTALL_DIR} )


# for the subdirs
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( ui )
add_subdirectory( tests )
add_subdirectory( plugins )


########### install files ###############

install( FILES spelling/sonnetspeller.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES 
        misc/backgroundchecker.h
        misc/defaultdictionary.h
        misc/filter.h
        misc/settings.h
        spelling/loader.h
        spelling/speller.h
DESTINATION ${INCLUDE_INSTALL_DIR}/sonnet)

