
project(kde3support)

# inter-subdir dependencies
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/kdecore ${CMAKE_CURRENT_SOURCE_DIR}/kdeui )
#${CMAKE_CURRENT_SOURCE_DIR}/kparts ${CMAKE_CURRENT_SOURCE_DIR}/kutils )

# all dependent libs
include_directories( ${KDE4_KPARTS_INCLUDES} )

add_subdirectory( kdeui )
add_subdirectory( kio )
add_subdirectory( kparts )

if(UNIX)
   add_subdirectory( kunittest )
   add_subdirectory( tests )
endif(UNIX)

add_definitions(-DKDE3_SUPPORT)

# kde3support/kdecore/Makefile.am: kde3core

set(kde3core_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kde3support/kdecore/k3urldrag.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdecore/k3multipledrag.cpp
)

install_files( /include FILES kdecore/k3urldrag.h kdecore/k3multipledrag.h kdecore/k3sqlpropertymap.h )

# kde3support/kdeui/Makefile.am: kde3ui

set(kde3ui_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3dockwidget.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3dockwidget_private.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3popupmenu.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3textedit.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3colordrag.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3syntaxhighlighter.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3wizard.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3spell.cpp  
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3spelldlg.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3sconfig.cpp
)

set(kde3ui_UI
    ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3spellui.ui
)

kde4_add_ui_files(kde3ui_STAT_SRCS ${kde3ui_UI} )

# kde3support/kio/Makefile.am: k3io

set(k3io_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kde3support/kio/k3bookmarkdrag.cpp
)

# kde3support/kparts/Makefile.am: k3parts

set(k3parts_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kde3support/kparts/dockmainwindow3.cpp
)

########### next target ###############

set(kde3support_LIB_SRCS
${k3parts_STAT_SRCS}
${kde3core_STAT_SRCS}
${kde3ui_STAT_SRCS}
${k3io_STAT_SRCS}
)

remove_definitions(-DQT3_SUPPORT_WARNINGS -DKDE_DEPRECATED_WARNINGS)
kde4_automoc(${kde3support_LIB_SRCS})

kde4_add_library(kde3support SHARED ${kde3support_LIB_SRCS})

target_link_libraries(kde3support  ${KDE4_KDECORE_LIBS}  ${QT_QTXML_LIBRARY} kdeui kutils kparts kio)

set_target_properties(kde3support PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(${LIB_INSTALL_DIR} kde3support )


########### install files ###############


