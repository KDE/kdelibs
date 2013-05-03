project(kdeui)

# QUrl
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

#set(KDEUI_NO_ATTICA FALSE)
#if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
#    set(KDEUI_NO_ATTICA TRUE)
#endif(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
# TODO QT5 Make the above work and port it to whatever parts of QtGui actually need attica.
set(KDEUI_NO_ATTICA TRUE)

include_directories(
 ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor
 ${DBUSMENUQT_INCLUDE_DIR}
)

if(NOT KDEUI_NO_ATTICA)
   include_directories( ${LIBATTICA_INCLUDE_DIR} )
   set(KDEUI_EXTRA_LIBS ${KDEUI_EXTRA_LIBS} ${LIBATTICA_LIBRARIES} )
   set(HAVE_ATTICA TRUE)
endif(NOT KDEUI_NO_ATTICA)

macro_bool_to_01(HAVE_ATTICA)


find_package(X11)
if(X11_FOUND)
	macro_bool_to_01(X11_FOUND HAVE_X11)
	macro_log_feature(X11_Xrender_FOUND "X Rendering Extension (libXrender)" "Support for compositing, rendering operations, and alpha-blending" "http://www.x.org" FALSE "" "STRONGLY RECOMMENDED")
	macro_bool_to_01(X11_Xrender_FOUND HAVE_XRENDER)
	macro_bool_to_01(X11_XTest_FOUND HAVE_XTEST)
endif(X11_FOUND)

find_package(DBusMenuQt)
macro_log_feature(DBUSMENUQT_FOUND "DBusMenuQt" "Support for notification area menus via the DBusMenu protocol" "https://launchpad.net/libdbusmenu-qt" TRUE "" "")
macro_bool_to_01(DBUSMENUQT_FOUND HAVE_DBUSMENUQT)

check_include_files(sys/stat.h    HAVE_SYS_STAT_H)
check_include_files(paths.h       HAVE_PATHS_H)
check_include_files(limits.h      HAVE_LIMITS_H)
check_include_files(sys/types.h   HAVE_SYS_TYPES_H)
check_include_files(sys/time.h    HAVE_SYS_TIME_H)
check_include_files(unistd.h      HAVE_UNISTD_H)
check_symbol_exists(posix_madvise   "sys/mman.h"               HAVE_MADVISE)
check_function_exists(strlcpy    HAVE_STRLCPY)
check_prototype_exists(strlcpy string.h             HAVE_STRLCPY_PROTO)
check_include_files(string.h      HAVE_STRING_H)

configure_file(config-kdeui.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdeui.h )

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/kaboutapplicationconfigattica_p.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/dialogs/kaboutapplicationconfigattica_p.h
)

set(HAVE_KSECRETSSERVICE 0)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/util/config-kwallet.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/util/config-kwallet.h
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/util/config-strlcpy.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/util/config-strlcpy.h
)

add_subdirectory( about )
add_subdirectory( tests )
add_subdirectory( sonnet/tests )

########### next target ###############
set(kdeui_LIB_SRCS
 actions/kactioncollection.cpp
 actions/kactionconflictdetector.cpp
 actions/kactioncategory.cpp
 actions/kactionmenu.cpp
 actions/kcodecaction.cpp
 actions/kdualaction.cpp
 actions/kfontaction.cpp
 actions/kfontsizeaction.cpp
 actions/kpastetextaction.cpp
 actions/krecentfilesaction.cpp
 actions/kselectaction.cpp
 actions/kstandardaction.cpp
 actions/ktoggleaction.cpp
 actions/ktoggletoolbaraction.cpp
 actions/ktogglefullscreenaction.cpp
 actions/ktoolbarlabelaction.cpp
 actions/ktoolbarpopupaction.cpp
 actions/ktoolbarspaceraction.cpp
 colors/kcolorbutton.cpp
 colors/kcolorcombo.cpp
 dialogs/kaboutkdedialog_p.cpp
 dialogs/kassistantdialog.cpp
 dialogs/kconfigdialog.cpp
 dialogs/kconfigdialogmanager.cpp
 dialogs/kbugreport.cpp
 dialogs/kedittoolbar.cpp
 dialogs/kinputdialog.cpp
 dialogs/klinkdialog.cpp
 dialogs/kaboutapplicationdialog.cpp
 dialogs/kaboutapplicationpersonmodel_p.cpp
 dialogs/kaboutapplicationpersonlistdelegate_p.cpp
 dialogs/kaboutapplicationpersonlistview_p.cpp
 dialogs/knewpassworddialog.cpp
 dialogs/kpassworddialog.cpp
 dialogs/kpixmapregionselectordialog.cpp
 dialogs/kshortcuteditwidget.cpp
 dialogs/kshortcutseditordelegate.cpp
 dialogs/kshortcutseditoritem.cpp
 dialogs/kshortcutseditor.cpp
 dialogs/kshortcutschemeseditor.cpp
 dialogs/kshortcutsdialog.cpp
 dialogs/kswitchlanguagedialog_p.cpp
 dialogs/ktip.cpp
 findreplace/kfind.cpp
 findreplace/kreplace.cpp
 findreplace/kfinddialog.cpp
 findreplace/kreplacedialog.cpp
 fonts/fonthelpers.cpp
 fonts/kfontchooser.cpp
 fonts/kfontcombobox.cpp
 fonts/kfontdialog.cpp
 fonts/kfontrequester.cpp
 fonts/kfontutils.cpp
 icons/kiconbutton.cpp
 icons/kicondialog.cpp
 itemviews/klistwidgetsearchline.cpp
 itemviews/ktreewidgetsearchline.cpp
 itemviews/kviewstatemaintainer.cpp
 itemviews/kfilterproxysearchline.cpp
 itemviews/kcategorizedsortfilterproxymodel.cpp
 itemviews/kcategorizedview.cpp
 itemviews/kcategorydrawer.cpp
 itemviews/kwidgetitemdelegate.cpp
 itemviews/kwidgetitemdelegatepool.cpp
 # This requires patches which are not yet in Qt. Disabled for now.
#  itemviews/kreparentingproxymodel.cpp
 jobs/kdialogjobuidelegate.cpp
 jobs/kabstractwidgetjobtracker.cpp
 jobs/kwidgetjobtracker.cpp
 jobs/kstatusbarjobtracker.cpp
 jobs/kjobtrackerformatters.cpp
 kernel/kstyle.cpp
 notifications/knotificationrestrictions.cpp
 paged/kpagedialog.cpp
 paged/kpagemodel.cpp
 paged/kpageview.cpp
 paged/kpageview_p.cpp
 paged/kpagewidget.cpp
 paged/kpagewidgetmodel.cpp
 shortcuts/kshortcutschemeshelper.cpp
 shortcuts/kglobalaccel.cpp
 shortcuts/kglobalshortcutinfo.cpp
 shortcuts/kglobalshortcutinfo_dbus.cpp
 shortcuts/kacceleratormanager.cpp
 shortcuts/kcheckaccelerators.cpp
 shortcuts/kgesture.cpp
 shortcuts/kgesturemap.cpp
 util/strlcpy-fake.c
 util/kckey.cpp
 util/kcompletion.cpp
 util/kcompletionbase.cpp
 util/kcrash.cpp
 util/kcursor.cpp
 util/kimagecache.cpp
 util/kkeyserver.cpp
 util/kmodifierkeyinfo.cpp
 util/kmodifierkeyinfoprovider.cpp
 util/kpassivepopup.cpp
 util/kpassivepopupmessagehandler.cpp
 util/kpixmapcache.cpp
 util/kundoactions.cpp
 util/kpixmapsequence.cpp
 util/kpixmapsequenceoverlaypainter.cpp
 util/kpixmapsequencewidget.cpp
 util/kzoneallocator.cpp
 widgets/kactionselector.cpp
 widgets/kbuttongroup.cpp
 widgets/kcharselect.cpp
 widgets/kcharselectdata.cpp
 widgets/kcmodule.cpp
 widgets/kcombobox.cpp
 widgets/kcompletionbox.cpp
 widgets/kdatecombobox.cpp
 widgets/kdatepicker.cpp
 widgets/kdatetable.cpp
 widgets/kdatetimeedit.cpp
 widgets/kdatetimewidget.cpp
 widgets/kdatewidget.cpp
 widgets/keditlistwidget.cpp
 widgets/khelpmenu.cpp
 widgets/khistorycombobox.cpp
 widgets/kkeysequencewidget.cpp
 widgets/klanguagebutton.cpp
 widgets/klineedit.cpp
 widgets/kmessagewidget.cpp
 widgets/kmultitabbar.cpp
 widgets/knuminput.cpp
 widgets/kpixmapregionselectorwidget.cpp
 widgets/kratingpainter.cpp
 widgets/kratingwidget.cpp
 widgets/krichtextedit.cpp
 widgets/krichtextwidget.cpp
 widgets/kshortcutwidget.cpp
 widgets/ksqueezedtextlabel.cpp
 widgets/kstringvalidator.cpp
 widgets/ktabbar.cpp
 widgets/ktabwidget.cpp
 widgets/ktextbrowser.cpp
 widgets/ktextedit.cpp
 widgets/ktimecombobox.cpp
 widgets/ktimezonewidget.cpp
 widgets/ktitlewidget.cpp
 widgets/kdeuiwidgetsproxystyle.cpp
 widgets/nestedlisthelper.cpp
 xmlgui/kxmlguibuilder.cpp
 xmlgui/kxmlguiwindow.cpp
 xmlgui/kmainwindowiface.cpp
 xmlgui/kmenumenuhandler_p.cpp
 xmlgui/ktoolbarhandler.cpp
 xmlgui/kxmlguiclient.cpp
 xmlgui/kxmlguifactory.cpp
 xmlgui/kxmlguifactory_p.cpp
 xmlgui/kxmlguiversionhandler.cpp
 xmlgui/kmainwindow.cpp
 xmlgui/ktoolbar.cpp
)

if(NOT KDE_NO_DEPRECATED)
set(kdeui_LIB_SRCS
 ${kdeui_LIB_SRCS}
 widgets/keditlistbox.cpp
)
endif(NOT KDE_NO_DEPRECATED)

option(MAC_USE_OSXKEYCHAIN "On OS X, use the keychain as backend for kwallet, instead of kwalletd.")

if (APPLE AND MAC_USE_OSXKEYCHAIN)
    FIND_LIBRARY(SECURITY_LIBRARY Security)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kwallet_mac.cpp)
else(APPLE AND MAC_USE_OSXKEYCHAIN)
     set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kwallet.cpp)
else(APPLE AND MAC_USE_OSXKEYCHAIN)
  set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kwallet.cpp)
endif(APPLE AND MAC_USE_OSXKEYCHAIN)

if(NOT WINCE)
set(kdeui_LIB_SRCS
 ${kdeui_LIB_SRCS}
 dialogs/kdeprintdialog.cpp
 notifications/kstatusnotifieritem.cpp
 notifications/kstatusnotifieritemdbus_p.cpp
)
endif(NOT WINCE)



if (X11_FOUND)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kkeyserver_x11.cpp
                                         util/kxerrorhandler.cpp
                                         dialogs/kcupsoptionswidget_p.cpp
                                         dialogs/kcupsoptionsjobwidget_p.cpp
                                         dialogs/kcupsoptionspageswidget_p.cpp
                                         dialogs/kcupsoptionssettingswidget_p.cpp )
endif (X11_FOUND)
if (APPLE)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kkeyserver_mac.cpp )
endif (APPLE)
if (WIN32)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kkeyserver_win.cpp )
endif (WIN32)

message("Port to Qt5 native filter")
#if (X11_FOUND AND X11_Xkb_FOUND)
#    include_directories (
#        ${X11_Xkb_INCLUDE_PATH}
#        ${X11_Xlib_INCLUDE_PATH}
#    )
#    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kmodifierkeyinfoprovider_x11.cpp)
#else (X11_FOUND AND X11_Xkb_FOUND)
    set (kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kmodifierkeyinfoprovider_dummy.cpp)
#endif (X11_FOUND AND X11_Xkb_FOUND)

set( kdeui_UI
   dialogs/kpassworddialog.ui
   dialogs/knewpassworddialog.ui
   dialogs/kshortcutsdialog.ui
   widgets/kdatetimeedit.ui
   widgets/kshortcutwidget.ui
   )

if (X11_FOUND)
    set( kdeui_UI ${kdeui_UI} dialogs/kcupsoptionspageswidget.ui
                              dialogs/kcupsoptionsjobwidget.ui
    )
endif (X11_FOUND)

kde4_add_ui_files(kdeui_LIB_SRCS ${kdeui_UI} )


set(kwallet_xml util/org.kde.KWallet.xml)
install(FILES ${kwallet_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

qt5_add_dbus_interface( kdeui_LIB_SRCS ${kwallet_xml} kwallet_interface )

set(statusnotifierwatcher_xml ${CMAKE_CURRENT_SOURCE_DIR}/notifications/org.kde.StatusNotifierWatcher.xml)
qt5_add_dbus_interface(kdeui_LIB_SRCS ${statusnotifierwatcher_xml} statusnotifierwatcher_interface)
set(notifications_xml ${CMAKE_CURRENT_SOURCE_DIR}/notifications/org.freedesktop.Notifications.xml)
qt5_add_dbus_interface(kdeui_LIB_SRCS ${notifications_xml} notifications_interface)

if(NOT WINCE)
qt5_add_dbus_adaptor(kdeui_LIB_SRCS     ${CMAKE_CURRENT_SOURCE_DIR}/notifications/org.kde.StatusNotifierItem.xml
                     ${CMAKE_CURRENT_SOURCE_DIR}/notifications/kstatusnotifieritemdbus_p.h KStatusNotifierItemDBus)
endif(NOT WINCE)

set(kglobalaccel_xml ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts/org.kde.KGlobalAccel.xml)
set_source_files_properties(${kglobalaccel_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt5_add_dbus_interface(kdeui_LIB_SRCS ${kglobalaccel_xml} kglobalaccel_interface )
install(FILES ${kglobalaccel_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

set(kglobalaccel_component_xml ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts/org.kde.kglobalaccel.Component.xml)
set_source_files_properties(${kglobalaccel_component_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt5_add_dbus_interface(kdeui_LIB_SRCS ${kglobalaccel_component_xml} kglobalaccel_component_interface )
install(FILES ${kglobalaccel_component_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

kde4_add_library(kdeui ${LIBRARY_TYPE} ${kdeui_LIB_SRCS})
generate_export_header(kdeui)

target_include_directories(kdeui PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

set(kdeui_INCLUDES
 ${CMAKE_CURRENT_SOURCE_DIR}/actions
 ${CMAKE_CURRENT_SOURCE_DIR}/colors
 ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
 ${CMAKE_CURRENT_SOURCE_DIR}/findreplace
 ${CMAKE_CURRENT_SOURCE_DIR}/fonts
 ${CMAKE_CURRENT_SOURCE_DIR}/icons
 ${CMAKE_CURRENT_SOURCE_DIR}/itemviews
 ${CMAKE_CURRENT_SOURCE_DIR}/jobs
 ${CMAKE_CURRENT_SOURCE_DIR}/kernel
 ${CMAKE_CURRENT_SOURCE_DIR}/notifications
 ${CMAKE_CURRENT_SOURCE_DIR}/paged
 ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts
 ${CMAKE_CURRENT_SOURCE_DIR}/util
 ${CMAKE_CURRENT_SOURCE_DIR}/widgets
 ${CMAKE_CURRENT_SOURCE_DIR}/xmlgui
 ${CMAKE_CURRENT_BINARY_DIR}/util
 )
target_include_directories(kdeui PUBLIC "$<BUILD_INTERFACE:${kdeui_INCLUDES}>")

find_package(Qt5PrintSupport REQUIRED NO_MODULE)

target_link_libraries(kdeui LINK_PUBLIC ${ItemModels_LIBRARIES} sonnetcore sonnetui ${KAuth_LIBRARIES} kguiaddons kwidgets ${KWidgetsAddons_LIBRARIES} ki18n ItemViews KCompletion KNotifications KJobWidgets KConfigWidgets KIconThemes ${KDE4_KDECORE_LIBS} ${X11_LIBRARIES} Qt5::Widgets Qt5::Svg Qt5::PrintSupport
                            Qt5::Xml ${KDEUI_EXTRA_LIBS} Qt5::Network ${KCoreAddons_LIBRARIES} ${KWindowSystem_LIBRARIES} ${kdeqt5staging_LIBRARIES} ${KConfig_GUI_LIBRARY} Qt5::Concurrent)

if(X11_FOUND)
  find_package(Qt5X11Extras REQUIRED NO_MODULE)
  target_link_libraries(kdeui LINK_PUBLIC Qt5::X11Extras)
endif()

if(NOT WINCE)
target_link_libraries(kdeui  Qt5::Svg)
endif(NOT WINCE)

if(HAVE_DBUSMENUQT)
  target_link_libraries(kdeui ${DBUSMENUQT_LIBRARIES})
endif()

if(MINGW)
   target_link_libraries(kdeui gdi32 oleaut32)
endif(MINGW)

if(APPLE)
   target_link_libraries(kdeui ${CARBON_LIBRARY})
   if (MAC_USE_OSXKEYCHAIN)
     target_link_libraries(kdeui ${SECURITY_LIBRARY})
   endif (MAC_USE_OSXKEYCHAIN)
endif(APPLE)

if(X11_XTest_FOUND)
   target_link_libraries(kdeui ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

if(X11_Xcursor_FOUND)
   target_link_libraries(kdeui ${X11_Xcursor_LIB})
endif(X11_Xcursor_FOUND)

if (X11_Xfixes_FOUND)
   target_link_libraries(kdeui ${X11_Xfixes_LIB})
endif(X11_Xfixes_FOUND)

if (X11_Xrender_FOUND)
   target_link_libraries(kdeui ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

target_link_libraries(kdeui LINK_INTERFACE_LIBRARIES ${KConfig_GUI_LIBRARY} Qt5::Widgets)

if(NOT WINCE)
target_link_libraries(kdeui LINK_INTERFACE_LIBRARIES Qt5::Svg)
endif(NOT WINCE)

set_target_properties(kdeui PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION}
                                       SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
                       )

install(TARGETS kdeui EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
########### install files ###############

if (APPLE)
   install( FILES util/kkeyserver_mac.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
elseif (WIN32)
    install( FILES util/kkeyserver_win.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )
else(WIN32)
   install( FILES util/kkeyserver_x11.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
endif (APPLE)

install( FILES
 dialogs/aboutkde.png
 dialogs/kdeprint_nup1.png
 dialogs/kdeprint_nup2.png
 dialogs/kdeprint_nup4.png
 dialogs/kdeprint_nupother.png
 dialogs/ktip-bulb.png
 dialogs/thumb_frame.png
 DESTINATION  ${DATA_INSTALL_DIR}/kdeui/pics )

install( FILES widgets/kcharselect-data  DESTINATION  ${DATA_INSTALL_DIR}/kcharselect )

install( FILES xmlgui/ui_standards.rc  DESTINATION  ${CONFIG_INSTALL_DIR}/ui )

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kdeui_export.h
 actions/kactioncollection.h
 actions/kactioncategory.h
 actions/kactionmenu.h
 actions/kdualaction.h
 actions/kcodecaction.h
 actions/kfontaction.h
 actions/kfontsizeaction.h
 actions/kpastetextaction.h
 actions/krecentfilesaction.h
 actions/kselectaction.h
 actions/kstandardaction.h
 actions/ktoggleaction.h
 actions/ktoggletoolbaraction.h
 actions/ktogglefullscreenaction.h
 actions/ktoolbarlabelaction.h
 actions/ktoolbarpopupaction.h
 actions/ktoolbarspaceraction.h
 colors/kcolorbutton.h
 colors/kcolorcombo.h
 dialogs/kaboutapplicationdialog.h
 dialogs/kassistantdialog.h
 dialogs/kbugreport.h
 dialogs/kconfigdialog.h
 dialogs/kconfigdialogmanager.h
 dialogs/kdeprintdialog.h
 dialogs/kedittoolbar.h
 dialogs/kinputdialog.h
 dialogs/knewpassworddialog.h
 dialogs/kpassworddialog.h
 dialogs/kpixmapregionselectordialog.h
 dialogs/kshortcutsdialog.h
 dialogs/kshortcutseditor.h
 dialogs/ktip.h
 findreplace/kfind.h
 findreplace/kreplace.h
 findreplace/kfinddialog.h
 findreplace/kreplacedialog.h
 fonts/kfontchooser.h
 fonts/kfontcombobox.h
 fonts/kfontdialog.h
 fonts/kfontrequester.h
 fonts/kfontutils.h
 icons/kiconbutton.h
 icons/kicondialog.h
 itemviews/kviewstatesaver.h
 itemviews/kviewstatemaintainer.h
 itemviews/klistwidgetsearchline.h
 itemviews/ktreewidgetsearchline.h
 itemviews/kfilterproxysearchline.h
 itemviews/kcategorizedsortfilterproxymodel.h
 itemviews/kcategorizedview.h
 itemviews/kcategorydrawer.h
 itemviews/kwidgetitemdelegate.h
#  itemviews/kreparentingproxymodel.h
 jobs/kdialogjobuidelegate.h
 jobs/kabstractwidgetjobtracker.h
 jobs/kwidgetjobtracker.h
 jobs/kstatusbarjobtracker.h
 kernel/kstyle.h
 notifications/kstatusnotifieritem.h
 notifications/knotificationrestrictions.h
 paged/kpagedialog.h
 paged/kpagemodel.h
 paged/kpageview.h
 paged/kpagewidget.h
 paged/kpagewidgetmodel.h
 shortcuts/kglobalaccel.h
 shortcuts/kglobalshortcutinfo.h
 shortcuts/kglobalshortcutinfo_p.h
 shortcuts/kacceleratormanager.h
 shortcuts/kgesture.h
 util/kaccelgen.h
 util/kcompletion.h
 util/kcrash.h
 util/kcursor.h
 util/kkeyserver.h
 util/kmodifierkeyinfo.h
 util/kpassivepopup.h
 util/kpassivepopupmessagehandler.h
 util/kpixmapcache.h
 util/kimagecache.h
 util/kwallet.h
 util/kxerrorhandler.h
 util/kundoactions.h
 util/kpixmapsequence.h
 util/kpixmapsequenceoverlaypainter.h
 util/kpixmapsequencewidget.h
 widgets/kactionselector.h
 widgets/kdatecombobox.h
 widgets/kdatepicker.h
 widgets/kcmodule.h
 widgets/kcombobox.h
 widgets/kcompletionbox.h
 widgets/kbuttongroup.h
 widgets/kcharselect.h
 widgets/kdatetable.h
 widgets/kdatetimeedit.h
 widgets/kdatetimewidget.h
 widgets/kdatewidget.h
 widgets/keditlistwidget.h
 widgets/khelpmenu.h
 widgets/khistorycombobox.h
 widgets/kkeysequencewidget.h
 widgets/klanguagebutton.h
 widgets/klineedit.h
 widgets/kmessagewidget.h
 widgets/kmultitabbar.h
 widgets/knuminput.h
 widgets/kpixmapregionselectorwidget.h
 widgets/kratingpainter.h
 widgets/kratingwidget.h
 widgets/krichtextedit.h
 widgets/krichtextwidget.h
 widgets/ksqueezedtextlabel.h
 widgets/kstringvalidator.h
 widgets/ktextbrowser.h
 widgets/ktextedit.h
 widgets/ktimecombobox.h
 widgets/ktimezonewidget.h
 widgets/ktitlewidget.h
 widgets/ktabbar.h
 widgets/ktabwidget.h
 widgets/kshortcutwidget.h
 xmlgui/kxmlguiwindow.h
 xmlgui/kxmlguiclient.h
 xmlgui/kxmlguibuilder.h
 xmlgui/kxmlguifactory.h
 xmlgui/kmainwindow.h
 xmlgui/ktoolbar.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

if(NOT KDE_NO_DEPRECATED)
install(FILES
 widgets/keditlistbox.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )
endif(NOT KDE_NO_DEPRECATED)

install(PROGRAMS preparetips DESTINATION ${BIN_INSTALL_DIR} )

if (APPLE)
  install(PROGRAMS kernel/start-session-bus.sh DESTINATION ${BIN_INSTALL_DIR} )
endif (APPLE)

install(FILES
    notifications/org.kde.StatusNotifierItem.xml
    notifications/org.kde.StatusNotifierWatcher.xml
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

install( FILES
    KDEUIMacros.cmake
    kde4uic.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )
