
project(kspell2)

include_directories( ${KDE4_KPARTS_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/kutils )

# kspell2/Makefile.am: kspell2base

set(kspell2base_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kspell2/settings.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/loader.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/client.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/filter.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/backgroundchecker.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/backgroundthread.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/backgroundengine.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/defaultdictionary.cpp
)

# for the subdirs
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( ui )
add_subdirectory( tests )
add_subdirectory( plugins )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES kspellclient.desktop )
install_files( /include/kspell2 FILES settings.h filter.h   dictionary.h loader.h   backgroundchecker.h defaultdictionary.h  )




