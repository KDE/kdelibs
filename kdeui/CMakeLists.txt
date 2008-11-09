
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
 ${CMAKE_CURRENT_SOURCE_DIR}/paged
 ${CMAKE_CURRENT_SOURCE_DIR}/plotting
 ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts
 ${CMAKE_CURRENT_SOURCE_DIR}/sonnet
 ${CMAKE_CURRENT_SOURCE_DIR}/util
 ${CMAKE_CURRENT_SOURCE_DIR}/widgets
 ${CMAKE_CURRENT_SOURCE_DIR}/windowmanagement
 ${CMAKE_CURRENT_SOURCE_DIR}/xmlgui
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
 colors/kcolorbutton.cpp
 colors/kcolorcombo.cpp
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
 dialogs/kdeprintdialog.cpp
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
 itemviews/klistwidget.cpp
 itemviews/klistwidgetsearchline.cpp
 itemviews/ktreewidgetsearchline.cpp
 itemviews/kfilterproxysearchline.cpp
 itemviews/kextendableitemdelegate.cpp
 itemviews/kcategorizedsortfilterproxymodel.cpp
 itemviews/kcategorizedview.cpp
 itemviews/kcategorydrawer.cpp
 itemviews/kwidgetitemdelegate.cpp
 itemviews/kwidgetitemdelegatepool.cpp
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
 shortcuts/kdedglobalaccel_interface.cpp
 shortcuts/kdedglobalaccel_component_interface.cpp
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
 util/kkeyserver.cpp
 util/kmanagerselection.cpp
 util/knotification.cpp
 util/knotificationmanager.cpp
 util/knotificationrestrictions.cpp
 util/knumvalidator.cpp
 util/kpassivepopup.cpp
 util/kpassivepopupmessagehandler.cpp
 util/kpixmapcache.cpp
 util/kpixmapprovider.cpp
 util/kstandardguiitem.cpp
 util/ksvgrenderer.cpp
 util/ksystemtrayicon.cpp
 util/kwallet.cpp
 util/kwhatsthismanager.cpp
 util/kwordwrap.cpp
 util/kxerrorhandler.cpp
 util/kxmessages.cpp
 util/kxutils.cpp
 util/kundostack.cpp
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

if (Q_WS_X11)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kkeyserver_x11.cpp
                                         dialogs/kcupsoptionswidget_p.cpp
                                         dialogs/kcupsoptionsjobwidget_p.cpp
                                         dialogs/kcupsoptionspageswidget_p.cpp )
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

if(MINGW)
add_definitions(-D_WIN32_WINNT=0x0500 -DWINVER=0x0500)
endif(MINGW)

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

kde4_add_library(kdeui SHARED ${kdeui_LIB_SRCS})

target_link_libraries(kdeui  ${QT_QTSVG_LIBRARY} ${KDE4_KDECORE_LIBS} ${X11_LIBRARIES} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY})

if(MINGW)
   target_link_libraries(kdeui gdi32 oleaut32)
endif(MINGW)

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

set_target_properties(kdeui PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} 
                                       SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION} 
                                       ${KDE4_DISABLE_PROPERTY_}LINK_INTERFACE_LIBRARIES "kdecore;${QT_QTSVG_LIBRARY};${QT_QTGUI_LIBRARY}"
                       )

install(TARGETS kdeui ${INSTALL_TARGETS_DEFAULT_ARGS})
########### install files ###############

if (Q_WS_MAC)
   install( FILES util/kkeyserver_mac.h DESTINATION ${INCLUDE_INSTALL_DIR} )
elseif (Q_WS_WIN)
    install( FILES util/kkeyserver_win.h DESTINATION ${INCLUDE_INSTALL_DIR} )
else(Q_WS_WIN)
   install( FILES util/fixx11h.h util/kkeyserver_x11.h DESTINATION ${INCLUDE_INSTALL_DIR} )
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
 itemviews/klistwidget.h
 itemviews/klistwidgetsearchline.h
 itemviews/ktreewidgetsearchline.h
 itemviews/kfilterproxysearchline.h
 itemviews/kextendableitemdelegate.h
 itemviews/kcategorizedsortfilterproxymodel.h
 itemviews/kcategorizedview.h
 itemviews/kcategorydrawer.h
 itemviews/kwidgetitemdelegate.h
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
 util/knotification.h
 util/knotificationrestrictions.h
 util/knumvalidator.h
 util/kpassivepopup.h
 util/kpassivepopupmessagehandler.h
 util/kpixmapcache.h
 util/kpixmapprovider.h
 util/kstandardguiitem.h
 util/ksvgrenderer.h
 util/ksystemtrayicon.h
 util/kwallet.h
 util/kwordwrap.h
 util/kxerrorhandler.h
 util/kxutils.h
 util/kxmessages.h
 util/kundostack.h
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
  DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
