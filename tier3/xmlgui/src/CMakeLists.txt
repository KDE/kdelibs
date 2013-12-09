add_subdirectory(ksendbugmail)

set(LibAttica_MIN_VERSION "1.0.0" CACHE STRING "The LibAttica Minimum version")
find_package(LibAttica ${LibAttica_MIN_VERSION} NO_MODULE)
set_package_properties(LibAttica PROPERTIES DESCRIPTION "A Qt library that implements the Open Collaboration Services API"
                       PURPOSE "Support for Get Hot New Stuff in XMLGUI"
                       URL "https://projects.kde.org/attica"
                       TYPE OPTIONAL
                      )

if (X11_FOUND)
   set (XMLGUI_EXTRA_LIBS ${XMLGUI_EXTRA_LIBS} ${X11_LIBRARIES})
endif ()

if (LibAttica_FOUND)
   set (HAVE_ATTICA 1 CACHE BOOL "TRUE if Attica was found")
   set (XMLGUI_EXTRA_LIBS ${XMLGUI_EXTRA_LIBS} ${LibAttica_LIBRARIES})
else ()
   set (HAVE_ATTICA 0 CACHE BOOL "TRUE if Attica was found")
endif ()

configure_file(config-xmlgui.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-xmlgui.h )

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/kaboutapplicationconfigattica_p.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/kaboutapplicationconfigattica_p.h
)

set(xmlgui_SRCS
  kaboutapplicationdialog.cpp
  kaboutapplicationpersonlistdelegate_p.cpp
  kaboutapplicationpersonlistview_p.cpp
  kaboutapplicationpersonmodel_p.cpp
  kaboutkdedialog_p.cpp
  kactioncategory.cpp
  kactioncollection.cpp
  kactionconflictdetector.cpp
  kbugreport.cpp
  kedittoolbar.cpp
  kgesture.cpp
  kgesturemap.cpp
  khelpmenu.cpp
  kkeysequencewidget.cpp
  kmainwindow.cpp
  kmainwindowiface.cpp
  kmenumenuhandler_p.cpp
  kshortcuteditwidget.cpp
  kshortcutschemeseditor.cpp
  kshortcutschemeshelper.cpp
  kshortcutsdialog.cpp
  kshortcutseditor.cpp
  kshortcutseditordelegate.cpp
  kshortcutseditoritem.cpp
  kshortcutwidget.cpp
  kswitchlanguagedialog_p.cpp
  ktoggletoolbaraction.cpp
  ktoolbar.cpp
  ktoolbarhandler.cpp
  kxmlguibuilder.cpp
  kxmlguiclient.cpp
  kxmlguifactory.cpp
  kxmlguifactory_p.cpp
  kxmlguiversionhandler.cpp
  kxmlguiwindow.cpp
  kundoactions.cpp
  kcheckaccelerators.cpp
  khelpclient.cpp
)

set( xmlgui_UI
   kshortcutsdialog.ui
   kshortcutwidget.ui
)

qt5_wrap_ui(xmlgui_SRCS ${xmlgui_UI} )

add_library(KF5XmlGui ${xmlgui_SRCS})
generate_export_header(KF5XmlGui BASE_NAME XmlGui)
add_library(KF5::XmlGui ALIAS KF5XmlGui)

target_link_libraries(KF5XmlGui
PUBLIC
    Qt5::DBus           #QDBus connect to signal in KToolBar
    Qt5::Xml            #To parse the configuration (QDomDocument etc)
    Qt5::Widgets        #QWidget is used everywhere
    KF5::ConfigCore         #Reading config for ToolbarIcons, Shortcut Schemes...
    KF5::KConfigWidgets      #KStandardAction, KToggleAction...
PRIVATE
    Qt5::Network        #QNetworkAccessManager in kaboutapplicationpersonmodel_p
    Qt5::PrintSupport   #QPrinter in kshortcutseditor
    Qt5::X11Extras      #QX11Info in kglobalaccel, QX11Info::appTime, appUserTime, setAppUserTime
    KF5::WidgetsAddons
    KF5::ItemViews           #KWidgetItemDelegate in KAboutApplicationPersonListDelegate
    KF5::I18n                #i18n and i18nc in many places
    KF5::KIconThemes         #KIconLoader and KIconThemes in KToolBar
    KF5::KTextWidgets        #KTextEdit in kbugreport
    KF5::WindowSystem       #KKeyServer in kkeysequencewidget
    KF5::GlobalAccel         #KActionCollection and shortcut widgets
    ${XMLGUI_EXTRA_LIBS}
)

set_target_properties(KF5XmlGui PROPERTIES VERSION   ${XMLGUI_VERSION_STRING}
                                           SOVERSION ${XMLGUI_SOVERSION}
                                           EXPORT_NAME XmlGui
)

install(TARGETS KF5XmlGui EXPORT KF5XmlGuiTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kaboutapplicationdialog.h
  kactioncategory.h
  kactioncollection.h
  kbugreport.h
  kedittoolbar.h
  kgesture_p.h
  kgesturemap_p.h
  khelpmenu.h
  kkeysequencewidget.h
  kmainwindow.h
  kshortcutsdialog.h
  kshortcutseditor.h
  kshortcutwidget.h
  ktoggletoolbaraction.h
  ktoolbar.h
  kxmlguibuilder.h
  kxmlguiclient.h
  kxmlguifactory.h
  kxmlguiwindow.h
  kundoactions.h
  khelpclient.h
  ${CMAKE_CURRENT_BINARY_DIR}/xmlgui_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES ui_standards.rc  DESTINATION  ${CONFIG_INSTALL_DIR}/ui )

install( FILES
 aboutkde.png
 thumb_frame.png
 DESTINATION  ${DATA_INSTALL_DIR}/xmlgui/pics )
