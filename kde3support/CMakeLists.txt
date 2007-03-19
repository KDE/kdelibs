
project(kde3support)
KDE4_NO_ENABLE_FINAL(kde3support)

# inter-subdir dependencies
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/kdecore ${CMAKE_CURRENT_SOURCE_DIR}/kdeui )
#${CMAKE_CURRENT_SOURCE_DIR}/kparts ${CMAKE_CURRENT_SOURCE_DIR}/kutils )

# all dependent libs
include_directories( ${KDE4_KPARTS_INCLUDES} )

add_definitions ( -DQT3_SUPPORT )

if(UNIX)
   add_subdirectory( kunittest )
   add_subdirectory( tests )
endif(UNIX)

add_definitions(-DKDE3_SUPPORT)

########### next target ###############

set(kde3support_LIB_SRCS
  kdecore/k3mimesourcefactory.cpp
  kdecore/k3multipledrag.cpp
  kdecore/k3rfcdate.cpp
  kdecore/k3tempfile.cpp
  kdecore/k3urldrag.cpp
  #kdeui/qxembed.cpp
  kdeui/k3aboutapplication.cpp
  kdeui/k3aboutdialog.cpp
  kdeui/k3activelabel.cpp
  kdeui/k3dockwidget.cpp
  kdeui/k3dockwidget_private.cpp
  kdeui/k3listbox.cpp
  kdeui/k3popupmenu.cpp
  kdeui/k3textedit.cpp
  kdeui/k3colordrag.cpp
  kdeui/k3syntaxhighlighter.cpp
  kdeui/k3wizard.cpp
  kdeui/k3spell.cpp
  kdeui/k3spelldlg.cpp
  kdeui/k3sconfig.cpp
  kdeui/k3passworddialog.cpp
  kio/k3bookmarkdrag.cpp
  kparts/dockmainwindow3.cpp
)

set(kde3ui_UI
    kdeui/k3spellui.ui
)

kde4_add_ui_files( kde3support_LIB_SRCS ${kde3ui_UI} )

remove_definitions(-DQT3_SUPPORT_WARNINGS -DKDE_DEPRECATED_WARNINGS)
kde4_automoc(${kde3support_LIB_SRCS})

kde4_add_library(kde3support SHARED ${kde3support_LIB_SRCS})

target_link_libraries(kde3support  ${KDE4_KDECORE_LIBS}  ${QT_QTXML_LIBRARY} kdeui kutils kparts kio)

set_target_properties(kde3support PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kde3support  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES
  kdecore/k3mimesourcefactory.h
  kdecore/k3multipledrag.h
  kdecore/k3rfcdate.h
  kdecore/k3sqlpropertymap.h
  kdecore/k3tempfile.h
  kdecore/k3urldrag.h
  kdecore/ksimpleconfig.h
  kdecore/ktrader.h
  kdeui/k3aboutapplication.h
  kdeui/k3aboutdialog.h
  kdeui/k3activelabel.h
  kdeui/k3dockwidget.h
  kdeui/k3listbox.h
  kdeui/k3popupmenu.h
  kdeui/k3textedit.h
  kdeui/k3colordrag.h
  kdeui/k3syntaxhighlighter.h
  kdeui/k3wizard.h
  kdeui/k3spell.h
  kdeui/k3spelldlg.h
  kdeui/k3sconfig.h
  kdeui/k3passworddialog.h
  kio/k3bookmarkdrag.h
  kio/kio/skipdlg.h
  kio/kio/passdlg.h
  kio/kfile/kurlrequesterdlg.h
  kio/kfile/kfilesharedlg.h
  kio/kfile/kpropsdlg.h
  kio/kssl/ksslinfodlg.h
  kio/kssl/ksslcertdlg.h
  kparts/dockmainwindow3.h
  kdeui/kactivelabel.h
  kdeui/kstdaccel.h
  kdeui/kstdaction.h
  kdeui/kstdguiitem.h
DESTINATION  ${INCLUDE_INSTALL_DIR} )

install( FILES
    kio/kio/renamedlg.h
DESTINATION  ${INCLUDE_INSTALL_DIR}/kio )
