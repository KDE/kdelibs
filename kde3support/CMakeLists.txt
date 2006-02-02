kde4_header()

add_subdirectory( kdecore ) 
add_subdirectory( kdeui ) 
add_subdirectory( kio ) 
add_subdirectory( kparts ) 
add_subdirectory( kmdi ) 
add_subdirectory( kunittest ) 
add_subdirectory( tests ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/kparts ${CMAKE_CURRENT_SOURCE_DIR}/kmdi ${CMAKE_CURRENT_SOURCE_DIR}/kmdi/res
${CMAKE_CURRENT_SOURCE_DIR}/kdeui ${CMAKE_SOURCE_DIR}/kutils ${CMAKE_CURRENT_SOURCE_DIR}/kdecore ${CMAKE_SOURCE_DIR}/kio/bookmarks)

add_definitions(-DKDE3_SUPPORT)

# kde3support/kdeui/Makefile.am: kde3ui

set(kde3ui_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3dockwidget.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3dockwidget_private.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3popupmenu.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3textedit.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3colordrag.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3syntaxhighlighter.cpp
)

# kde3support/kio/Makefile.am: k3io

set(k3io_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kde3support/kio/k3bookmarkdrag.cpp
)

# kde3support/kmdi/Makefile.am: k3mdi

set(k3mdi_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdichildarea.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdichildfrm.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdichildfrmcaption.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdichildview.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdimainfrm.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mditaskbar.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdidockcontainer.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mditoolviewaccessor.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdiguiclient.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdidocumentviewtabwidget.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdifocuslist.cpp
)

# kde3support/kparts/Makefile.am: k3parts

set(k3parts_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kde3support/kparts/dockmainwindow3.cpp
)

# kde3support/kdecore/Makefile.am: kde3core

set(kde3core_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kde3support/kdecore/k3shortcut.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdecore/k3keysequence.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdecore/k3shortcutmenu.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdecore/k3urldrag.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdecore/k3multipledrag.cpp
)


########### next target ###############

set(kde3support_LIB_SRCS
${k3mdi_STAT_SRCS}
${k3parts_STAT_SRCS}
${kde3core_STAT_SRCS}
${kde3ui_STAT_SRCS}
${k3io_STAT_SRCS}
)

kde4_automoc(${kde3support_LIB_SRCS})

kde4_add_library(kde3support SHARED ${kde3support_LIB_SRCS})

target_link_libraries(kde3support  ${QT_AND_KDECORE_LIBS}  ${QT_QTXML_LIBRARY} kdeui kutils kparts kio)

set_target_properties(kde3support PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(${KDE4_LIB_INSTALL_DIR} kde3support )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = kdecore kdeui kio kparts kmdi kunittest . tests
#lib_LTLIBRARIES = libkde3support.la
#
#libkde3support_la_SOURCES = dummy.cpp
#
#libkde3support_la_LDFLAGS = -no-undefined $(all_libraries) \
#        $(KDE_MT_LDFLAGS)
#libkde3support_la_LIBADD = ./kmdi/libk3mdi.la ./kparts/libk3parts.la ./kio/libk3io.la ./kdeui/libkde3ui.la ./kdecore/libkde3core.la
#
#dummy.cpp:
#	echo >dummy.cpp
