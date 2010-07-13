
project(kdeui)

include_directories(
 ${CMAKE_SOURCE_DIR}/interfaces
 ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor
 ${CMAKE_SOURCE_DIR}/kdecore/sonnet
 ${CMAKE_SOURCE_DIR}/kdeui
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}/actions
 ${CMAKE_CURRENT_SOURCE_DIR}/colors
 ${CMAKE_CURRENT_SOURCE_DIR}/config
 ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
 ${CMAKE_CURRENT_SOURCE_DIR}/findreplace
 ${CMAKE_CURRENT_SOURCE_DIR}/fonts
 ${CMAKE_CURRENT_SOURCE_DIR}/icons
 ${CMAKE_CURRENT_SOURCE_DIR}/itemviews
 ${CMAKE_CURRENT_SOURCE_DIR}/jobs
 ${CMAKE_CURRENT_SOURCE_DIR}/kernel
 ${CMAKE_CURRENT_SOURCE_DIR}/notifications
 ${CMAKE_CURRENT_SOURCE_DIR}/paged
 ${CMAKE_CURRENT_SOURCE_DIR}/plotting
 ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts
 ${CMAKE_CURRENT_SOURCE_DIR}/sonnet
 ${CMAKE_CURRENT_SOURCE_DIR}/util
 ${CMAKE_CURRENT_SOURCE_DIR}/widgets
 ${CMAKE_CURRENT_SOURCE_DIR}/windowmanagement
 ${CMAKE_CURRENT_SOURCE_DIR}/xmlgui
 ${DBUSMENUQT_INCLUDE_DIR}
)

add_subdirectory( about )
add_subdirectory( tests )
add_subdirectory( sonnet/tests )

########### next target ###############
set(kdeui_LIB_SRCS
 actions/kactioncollection.cpp
 actions/kactioncategory.cpp
 actions/kaction.cpp
 actions/kactionmenu.cpp
 actions/kcodecaction.cpp
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
 colors/kcolorspaces.cpp
 colors/kcolorutils.cpp
 colors/kcolorscheme.cpp
 colors/kcolorcollection.cpp
 colors/kcolorhelpers.cpp
 colors/kcolorbutton.cpp
 colors/kcolorcombo.cpp
 colors/kcolorchoosermode.cpp
 colors/kcolordialog.cpp
 colors/kcolormimedata.cpp
 colors/kcolorvalueselector.cpp
 colors/khuesaturationselect.cpp
 config/kconfigskeleton.cpp
 config/kconfiggroupgui.cpp
 dialogs/kaboutkdedialog_p.cpp
 dialogs/kassistantdialog.cpp
 dialogs/kconfigdialog.cpp
 dialogs/kconfigdialogmanager.cpp
 dialogs/kbugreport.cpp
 dialogs/kdialog.cpp
 dialogs/kedittoolbar.cpp
 dialogs/kinputdialog.cpp
 dialogs/klinkdialog.cpp
 dialogs/kmessagebox.cpp
 dialogs/kmessageboxmessagehandler.cpp
 dialogs/kaboutapplicationdialog.cpp
 dialogs/knewpassworddialog.cpp
 dialogs/kpassworddialog.cpp
 dialogs/kpixmapregionselectordialog.cpp
 dialogs/kprogressdialog.cpp
 dialogs/kshortcuteditwidget.cpp
 dialogs/kshortcutseditordelegate.cpp
 dialogs/kshortcutseditoritem.cpp
 dialogs/kshortcutseditor.cpp
 dialogs/kshortcutschemeseditor.cpp
 dialogs/kshortcutsdialog.cpp
 dialogs/ksplashscreen.cpp
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
 icons/kiconeffect.cpp
 icons/kiconengine.cpp
 icons/kicon.cpp
 icons/kiconloader.cpp
 icons/kicontheme.cpp
 icons/kiconcache.cpp
 itemviews/kbreadcrumbselectionmodel.cpp
 itemviews/klinkitemselectionmodel.cpp
 itemviews/krecursivefilterproxymodel.cpp
 itemviews/kdescendantsproxymodel.cpp
 itemviews/klistwidget.cpp
 itemviews/klistwidgetsearchline.cpp
 itemviews/ktreewidgetsearchline.cpp
 itemviews/kviewstatesaver.cpp
 itemviews/kfilterproxysearchline.cpp
 itemviews/kextendableitemdelegate.cpp
 itemviews/kcategorizedsortfilterproxymodel.cpp
 itemviews/kcategorizedview.cpp
 itemviews/kcategorydrawer.cpp
 itemviews/kmodelindexproxymapper.cpp
 itemviews/kwidgetitemdelegate.cpp
 itemviews/kwidgetitemdelegatepool.cpp
 # This requires patches which are not yet in Qt. Disabled for now.
#  itemviews/kreparentingproxymodel.cpp
 itemviews/kselectionproxymodel.cpp
 jobs/kdialogjobuidelegate.cpp
 jobs/kabstractwidgetjobtracker.cpp
 jobs/kwidgetjobtracker.cpp
 jobs/kstatusbarjobtracker.cpp
 jobs/kuiserverjobtracker.cpp
 kernel/kapplication.cpp
 kernel/kclipboard.cpp
 kernel/kuniqueapplication.cpp
 kernel/ksessionmanager.cpp
 kernel/kstyle.cpp
 kernel/kstartupinfo.cpp
 kernel/kglobalsettings.cpp
 notifications/kstatusnotifieritem.cpp
 notifications/kstatusnotifieritemdbus_p.cpp
 notifications/ksystemtrayicon.cpp
 notifications/knotification.cpp
 notifications/knotificationmanager.cpp
 notifications/knotificationrestrictions.cpp
 paged/kpagedialog.cpp
 paged/kpagemodel.cpp
 paged/kpageview.cpp
 paged/kpageview_p.cpp
 paged/kpagewidget.cpp
 paged/kpagewidgetmodel.cpp
 plotting/kplotaxis.cpp
 plotting/kplotpoint.cpp
 plotting/kplotobject.cpp
 plotting/kplotwidget.cpp
 shortcuts/kshortcut.cpp
 shortcuts/kshortcutschemeshelper.cpp
 shortcuts/kstandardshortcut.cpp
 shortcuts/kglobalaccel.cpp
 shortcuts/kglobalshortcutinfo.cpp
 shortcuts/kglobalshortcutinfo_dbus.cpp
 shortcuts/kacceleratormanager.cpp
 shortcuts/kcheckaccelerators.cpp
 shortcuts/kgesture.cpp
 shortcuts/kgesturemap.cpp
 sonnet/dialog.cpp
 sonnet/configwidget.cpp
 sonnet/highlighter.cpp
 sonnet/configdialog.cpp
 sonnet/dictionarycombobox.cpp
 util/kckey.cpp
 util/kcompletion.cpp
 util/kcompletionbase.cpp
 util/kcrash.cpp
 util/kcursor.cpp
 util/kguiitem.cpp
 util/kimagecache.cpp
 util/kkeyserver.cpp
 util/kmanagerselection.cpp
 util/kmodifierkeyinfo.cpp
 util/kmodifierkeyinfoprovider.cpp
 util/knumvalidator.cpp
 util/kpassivepopup.cpp
 util/kpassivepopupmessagehandler.cpp
 util/kpixmapcache.cpp
 util/kpixmapprovider.cpp
 util/kstandardguiitem.cpp
 util/ksvgrenderer.cpp
 util/kwallet.cpp
 util/kwordwrap.cpp
 util/kxerrorhandler.cpp
 util/kxmessages.cpp
 util/kxutils.cpp
 util/kundostack.cpp
 util/kpixmapsequence.cpp
 util/kpixmapsequenceoverlaypainter.cpp
 util/kpixmapsequencewidget.cpp
 widgets/kactionselector.cpp
 widgets/kanimatedbutton.cpp
 widgets/karrowbutton.cpp
 widgets/kdialogbuttonbox.cpp
 widgets/kbuttongroup.cpp
 widgets/kcapacitybar.cpp
 widgets/kcharselect.cpp
 widgets/kcharselectdata.cpp
 widgets/kcmodule.cpp
 widgets/kcombobox.cpp
 widgets/kcompletionbox.cpp
 widgets/kdatepicker.cpp
 widgets/kdatetable.cpp
 widgets/kdatetimewidget.cpp
 widgets/kdatewidget.cpp
 widgets/keditlistbox.cpp
 widgets/keditlistwidget.cpp
 widgets/kfadewidgeteffect.cpp
 widgets/khbox.cpp
 widgets/khelpmenu.cpp
 widgets/khistorycombobox.cpp
 widgets/kkeysequencewidget.cpp
 widgets/klanguagebutton.cpp
 widgets/kled.cpp
 widgets/klineedit.cpp
 widgets/kmainwindow.cpp
 widgets/kmenu.cpp
 widgets/kmenubar.cpp
 widgets/kmultitabbar.cpp
 widgets/knuminput.cpp
 widgets/kpixmapregionselectorwidget.cpp
 widgets/kpushbutton.cpp
 widgets/krestrictedline.cpp
 widgets/krichtextedit.cpp
 widgets/krichtextwidget.cpp
 widgets/kruler.cpp
 widgets/kselector.cpp
 widgets/kseparator.cpp
 widgets/kshortcutwidget.cpp
 widgets/ksqueezedtextlabel.cpp
 widgets/kstatusbar.cpp
 widgets/kstringvalidator.cpp
 widgets/ktabbar.cpp
 widgets/ktabwidget.cpp
 widgets/ktextbrowser.cpp
 widgets/ktextedit.cpp
 widgets/ktimezonewidget.cpp
 widgets/ktitlewidget.cpp
 widgets/ktoolbar.cpp
 widgets/kurllabel.cpp
 widgets/kvbox.cpp
 widgets/kxyselector.cpp
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
)

if(NOT WINCE)
set(kdeui_LIB_SRCS
 ${kdeui_LIB_SRCS}
 dialogs/kdeprintdialog.cpp
)
endif(NOT WINCE)

if (Q_WS_X11)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kkeyserver_x11.cpp
                                         dialogs/kcupsoptionswidget_p.cpp
                                         dialogs/kcupsoptionsjobwidget_p.cpp
                                         dialogs/kcupsoptionspageswidget_p.cpp
                                         dialogs/kcupsoptionssettingswidget_p.cpp )
endif (Q_WS_X11)
if (Q_WS_MAC)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kkeyserver_mac.cpp )
endif (Q_WS_MAC)
if (Q_WS_WIN)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} kernel/kapplication_win.cpp util/kkeyserver_win.cpp )
endif (Q_WS_WIN)


if (Q_WS_X11)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} windowmanagement/kwindowsystem_x11.cpp
                                         windowmanagement/kwindowinfo_x11.cpp
                                         windowmanagement/netwm.cpp )
endif (Q_WS_X11)
if (Q_WS_MAC)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} windowmanagement/kwindowsystem_mac.cpp
                                         windowmanagement/kwindowinfo_mac.cpp)
endif (Q_WS_MAC)
if (Q_WS_WIN)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} windowmanagement/kwindowsystem_win.cpp
                                         windowmanagement/kwindowinfo_win.cpp)
endif (Q_WS_WIN)
if (Q_WS_QWS)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} windowmanagement/kwindowsystem_qws.cpp)
endif (Q_WS_QWS)

if (Q_WS_X11 AND X11_Xkb_FOUND)
    include_directories (
        ${X11_Xkb_INCLUDE_PATH}
        ${X11_Xlib_INCLUDE_PATH}
    )
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kmodifierkeyinfoprovider_x11.cpp)
else (Q_WS_X11 AND X11_Xkb_FOUND)
    set (kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kmodifierkeyinfoprovider_dummy.cpp)
endif (Q_WS_X11 AND X11_Xkb_FOUND)

set( kdeui_UI
   dialogs/kpassworddialog.ui
   dialogs/knewpassworddialog.ui
   dialogs/kshortcutsdialog.ui
   widgets/kshortcutwidget.ui
   sonnet/sonnetui.ui
   sonnet/configui.ui
   )

if (Q_WS_X11)
    set( kdeui_UI ${kdeui_UI} dialogs/kcupsoptionspageswidget.ui
                              dialogs/kcupsoptionsjobwidget.ui
    )
endif (Q_WS_X11)

kde4_add_ui_files(kdeui_LIB_SRCS ${kdeui_UI} )


set(kwallet_xml util/org.kde.KWallet.xml)
install(FILES ${kwallet_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

qt4_add_dbus_interface( kdeui_LIB_SRCS ${kwallet_xml} kwallet_interface )
qt4_add_dbus_interfaces(kdeui_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/jobs/org.kde.JobViewServer.xml )

qt4_add_dbus_interface(kdeui_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/jobs/org.kde.JobView.xml jobviewiface )
qt4_add_dbus_interface(kdeui_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/jobs/org.kde.JobViewV2.xml jobviewifacev2 )

set(statusnotifierwatcher_xml ${CMAKE_CURRENT_SOURCE_DIR}/notifications/org.kde.StatusNotifierWatcher.xml)
qt4_add_dbus_interface(kdeui_LIB_SRCS ${statusnotifierwatcher_xml} statusnotifierwatcher_interface)
set(notifications_xml ${CMAKE_CURRENT_SOURCE_DIR}/notifications/org.freedesktop.Notifications.xml)
qt4_add_dbus_interface(kdeui_LIB_SRCS ${notifications_xml} notifications_interface)

qt4_add_dbus_adaptor(kdeui_LIB_SRCS     ${CMAKE_CURRENT_SOURCE_DIR}/notifications/org.kde.StatusNotifierItem.xml
                     ${CMAKE_CURRENT_SOURCE_DIR}/notifications/kstatusnotifieritemdbus_p.h KStatusNotifierItemDBus)

set(kglobalaccel_xml ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts/org.kde.KGlobalAccel.xml)
set_source_files_properties(${kglobalaccel_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt4_add_dbus_interface(kdeui_LIB_SRCS ${kglobalaccel_xml} kglobalaccel_interface )
install(FILES ${kglobalaccel_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

set(kglobalaccel_component_xml ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts/org.kde.kglobalaccel.Component.xml)
set_source_files_properties(${kglobalaccel_component_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt4_add_dbus_interface(kdeui_LIB_SRCS ${kglobalaccel_component_xml} kglobalaccel_component_interface )
install(FILES ${kglobalaccel_component_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

set(knotify_xml ${CMAKE_CURRENT_SOURCE_DIR}/notifications/org.kde.KNotify.xml)
qt4_add_dbus_interface(kdeui_LIB_SRCS ${knotify_xml} knotify_interface)

kde4_add_library(kdeui SHARED ${kdeui_LIB_SRCS})

target_link_libraries(kdeui  ${QT_QTSVG_LIBRARY} ${KDE4_KDECORE_LIBS} ${X11_LIBRARIES} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${DBUSMENUQT_LIBRARIES})

if(MINGW)
   target_link_libraries(kdeui gdi32 oleaut32)
endif(MINGW)

if(Q_WS_X11)
   target_link_libraries(kdeui ${QT_QTNETWORK_LIBRARY})
endif(Q_WS_X11)

if(Q_WS_MAC)
   target_link_libraries(kdeui ${CARBON_LIBRARY})
endif(Q_WS_MAC)

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

target_link_libraries(kdeui LINK_INTERFACE_LIBRARIES kdecore ${QT_QTSVG_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(kdeui PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION}
                                       SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
                       )

install(TARGETS kdeui EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
########### install files ###############

if (Q_WS_MAC)
   install( FILES util/kkeyserver_mac.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
elseif (Q_WS_WIN)
    install( FILES util/kkeyserver_win.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )
else(Q_WS_WIN)
   install( FILES util/fixx11h.h util/kkeyserver_x11.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
endif (Q_WS_MAC)

install( FILES
 dialogs/aboutkde.png
 dialogs/kdeprint_nup1.png
 dialogs/kdeprint_nup2.png
 dialogs/kdeprint_nup4.png
 dialogs/kdeprint_nupother.png
 dialogs/ktip-bulb.png
 DESTINATION  ${DATA_INSTALL_DIR}/kdeui/pics )

install( FILES widgets/kcharselect-data  DESTINATION  ${DATA_INSTALL_DIR}/kcharselect )

install( FILES xmlgui/ui_standards.rc  DESTINATION  ${CONFIG_INSTALL_DIR}/ui )

install( FILES
 kdeui_export.h
 actions/kactioncollection.h
 actions/kactioncategory.h
 actions/kaction.h
 actions/kactionmenu.h
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
 colors/kcolorutils.h
 colors/kcolorscheme.h
 colors/kcolorcollection.h
 colors/kcolorbutton.h
 colors/kcolorchoosermode.h
 colors/kcolorcombo.h
 colors/kcolordialog.h
 colors/kcolormimedata.h
 colors/kcolorvalueselector.h
 colors/khuesaturationselect.h
 config/kconfigskeleton.h
 dialogs/kaboutapplicationdialog.h
 dialogs/kassistantdialog.h
 dialogs/kbugreport.h
 dialogs/kconfigdialog.h
 dialogs/kconfigdialogmanager.h
 dialogs/kdeprintdialog.h
 dialogs/kdialog.h
 dialogs/kedittoolbar.h
 dialogs/kinputdialog.h
 dialogs/kmessagebox.h
 dialogs/kmessageboxmessagehandler.h
 dialogs/knewpassworddialog.h
 dialogs/kpassworddialog.h
 dialogs/kpixmapregionselectordialog.h
 dialogs/kprogressdialog.h
 dialogs/ksplashscreen.h
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
 icons/kiconeffect.h
 icons/kicon.h
 icons/k3icon_p.h      # we install the private header as it is needed by kcm (tokoe)
 icons/kiconloader.h
 icons/kicontheme.h
 icons/kiconcache.h
 itemviews/kbreadcrumbselectionmodel.h
 itemviews/klinkitemselectionmodel.h
 itemviews/krecursivefilterproxymodel.h
 itemviews/kdescendantsproxymodel.h
 itemviews/klistwidget.h
 itemviews/kviewstatesaver.h
 itemviews/klistwidgetsearchline.h
 itemviews/ktreewidgetsearchline.h
 itemviews/kfilterproxysearchline.h
 itemviews/kextendableitemdelegate.h
 itemviews/kcategorizedsortfilterproxymodel.h
 itemviews/kcategorizedview.h
 itemviews/kcategorydrawer.h
 itemviews/kmodelindexproxymapper.h
 itemviews/kwidgetitemdelegate.h
#  itemviews/kreparentingproxymodel.h
 itemviews/kselectionproxymodel.h
 jobs/kdialogjobuidelegate.h
 jobs/kabstractwidgetjobtracker.h
 jobs/kwidgetjobtracker.h
 jobs/kstatusbarjobtracker.h
 jobs/kuiserverjobtracker.h
 kernel/kapplication.h
 kernel/kuniqueapplication.h
 kernel/ksessionmanager.h
 kernel/kstyle.h
 kernel/kstartupinfo.h
 kernel/kglobalsettings.h
 notifications/kstatusnotifieritem.h
 notifications/ksystemtrayicon.h
 notifications/knotification.h
 notifications/knotificationrestrictions.h
 paged/kpagedialog.h
 paged/kpagemodel.h
 paged/kpageview.h
 paged/kpagewidget.h
 paged/kpagewidgetmodel.h
 plotting/kplotwidget.h
 plotting/kplotpoint.h
 plotting/kplotobject.h
 plotting/kplotaxis.h
 shortcuts/kshortcut.h
 shortcuts/kstandardshortcut.h
 shortcuts/kglobalaccel.h
 shortcuts/kglobalshortcutinfo.h
 shortcuts/kglobalshortcutinfo_p.h
 shortcuts/kacceleratormanager.h
 shortcuts/kgesture.h
 util/kaccelgen.h
 util/kcompletion.h
 util/kcrash.h
 util/kcursor.h
 util/kguiitem.h
 util/kkeyserver.h
 util/kmanagerselection.h
 util/kmodifierkeyinfo.h
 util/knumvalidator.h
 util/kpassivepopup.h
 util/kpassivepopupmessagehandler.h
 util/kpixmapcache.h
 util/kpixmapprovider.h
 util/kimagecache.h
 util/kstandardguiitem.h
 util/ksvgrenderer.h
 util/kwallet.h
 util/kwordwrap.h
 util/kxerrorhandler.h
 util/kxutils.h
 util/kxmessages.h
 util/kundostack.h
 util/kpixmapsequence.h
 util/kpixmapsequenceoverlaypainter.h
 util/kpixmapsequencewidget.h
 widgets/kactionselector.h
 widgets/kanimatedbutton.h
 widgets/karrowbutton.h
 widgets/kdatepicker.h
 widgets/kcapacitybar.h
 widgets/kcmodule.h
 widgets/kcombobox.h
 widgets/kcompletionbox.h
 widgets/kbuttongroup.h
 widgets/kcharselect.h
 widgets/kdatetable.h
 widgets/kdatetimewidget.h
 widgets/kdatewidget.h
 widgets/kdialogbuttonbox.h
 widgets/keditlistbox.h
 widgets/keditlistwidget.h
 widgets/kfadewidgeteffect.h
 widgets/khbox.h
 widgets/khelpmenu.h
 widgets/khistorycombobox.h
 widgets/kkeysequencewidget.h
 widgets/klanguagebutton.h
 widgets/kled.h
 widgets/klineedit.h
 widgets/kmainwindow.h
 widgets/kmenu.h
 widgets/kmenubar.h
 widgets/kmultitabbar.h
 widgets/knuminput.h
 widgets/kpixmapregionselectorwidget.h
 widgets/kpushbutton.h
 widgets/krestrictedline.h
 widgets/krichtextedit.h
 widgets/krichtextwidget.h
 widgets/kseparator.h
 widgets/ksqueezedtextlabel.h
 widgets/kstringvalidator.h
 widgets/ktextbrowser.h
 widgets/ktextedit.h
 widgets/ktimezonewidget.h
 widgets/ktitlewidget.h
 widgets/ktabbar.h
 widgets/ktabwidget.h
 widgets/kruler.h
 widgets/kselector.h
 widgets/kshortcutwidget.h
 widgets/kstatusbar.h
 widgets/ktoolbar.h
 widgets/kurllabel.h
 widgets/kvbox.h
 widgets/kxyselector.h
 windowmanagement/kwindowsystem.h
 windowmanagement/kwindowinfo.h
 windowmanagement/netwm_def.h
 windowmanagement/netwm.h
 xmlgui/kxmlguiwindow.h
 xmlgui/kxmlguiclient.h
 xmlgui/kxmlguibuilder.h
 xmlgui/kxmlguifactory.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

install( FILES
  sonnet/dialog.h
  sonnet/highlighter.h
  sonnet/configdialog.h
  sonnet/configwidget.h
  sonnet/dictionarycombobox.h
  DESTINATION  ${INCLUDE_INSTALL_DIR}/sonnet COMPONENT Devel )

install(PROGRAMS preparetips DESTINATION ${BIN_INSTALL_DIR} )

if (Q_WS_MAC)
  install(PROGRAMS kernel/start-session-bus.sh DESTINATION ${BIN_INSTALL_DIR} )
endif (Q_WS_MAC)

install( FILES colors/40.colors colors/Oxygen.colors colors/Web.colors colors/Royal.colors colors/Rainbow.colors
         DESTINATION ${CONFIG_INSTALL_DIR}/colors)

install( FILES
  jobs/org.kde.JobViewServer.xml
  jobs/org.kde.JobView.xml
  jobs/org.kde.JobViewV2.xml
  DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

install(FILES
    notifications/org.kde.StatusNotifierItem.xml
    notifications/org.kde.StatusNotifierWatcher.xml
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

if (NOT Q_WS_X11)
    # install our copy of the rgb.txt named colors list on systems without X11
    install(FILES colors/rgb.txt DESTINATION ${DATA_INSTALL_DIR}/kdeui)
endif (NOT Q_WS_X11)
