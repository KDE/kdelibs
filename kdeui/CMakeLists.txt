
project(kdeui)

include_directories(
 ${CMAKE_SOURCE_DIR}/kdefx
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
 colors/kcolorbutton.cpp
 colors/kcolorcombo.cpp
 colors/kcolordialog.cpp
 colors/kcolormimedata.cpp
 colors/kcolorvalueselector.cpp
 colors/khuesaturationselect.cpp
 config/kconfigskeleton.cpp
 config/kconfiggroupgui.cpp
 dialogs/kaboutkdedialog.cpp
 dialogs/kassistantdialog.cpp
 dialogs/kconfigdialog.cpp
 dialogs/kconfigdialogmanager.cpp
 dialogs/kbugreport.cpp
 dialogs/kdialog.cpp
 dialogs/kedittoolbar.cpp
 dialogs/kinputdialog.cpp
 dialogs/kmessagebox.cpp
 dialogs/kmessageboxmessagehandler.cpp
 dialogs/kaboutapplicationdialog.cpp
 dialogs/knewpassworddialog.cpp
 dialogs/kpassworddialog.cpp
 dialogs/kpixmapregionselectordialog.cpp
 dialogs/kprogressdialog.cpp
 dialogs/kshortcutsdialog.cpp
 dialogs/ksplashscreen.cpp
 dialogs/ktip.cpp
 findreplace/kfind.cpp
 findreplace/kreplace.cpp
 findreplace/kfinddialog.cpp
 findreplace/kreplacedialog.cpp
 fonts/kfontchooser.cpp
 fonts/kfontdialog.cpp
 fonts/kfontrequester.cpp
 icons/kiconeffect.cpp
 icons/kiconengine.cpp
 icons/kicon.cpp
 icons/kiconloader.cpp
 icons/kicontheme.cpp
 itemviews/klistwidget.cpp
 itemviews/klistwidgetsearchline.cpp
 itemviews/ktreewidgetsearchline.cpp
 itemviews/kextendableitemdelegate.cpp
 jobs/kdialogjobuidelegate.cpp
 jobs/kabstractwidgetjobtracker.cpp
 jobs/kwidgetjobtracker.cpp
 jobs/kstatusbarjobtracker.cpp
 jobs/kuiserverjobtracker.cpp
 kernel/kapplication.cpp
 kernel/kclipboard.cpp
 kernel/kuniqueapplication.cpp
 kernel/ksessionmanager.cpp
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
 shortcuts/kstandardshortcut.cpp
 shortcuts/kglobalaccel.cpp
 shortcuts/kacceleratormanager.cpp
 shortcuts/kcheckaccelerators.cpp
 shortcuts/kgesture.cpp
 shortcuts/kgesturemap.cpp
 sonnet/dialog.cpp
 sonnet/configwidget.cpp
 sonnet/highlighter.cpp
 sonnet/configdialog.cpp
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
 util/kpalette.cpp
 util/kpassivepopup.cpp
 util/kpassivepopupmessagehandler.cpp
 util/kpixmapprovider.cpp
 util/kstandardguiitem.cpp
 util/ksvgrenderer.cpp
 util/ksystemtrayicon.cpp
 util/kwhatsthismanager.cpp
 util/kwordwrap.cpp
 util/kxerrorhandler.cpp
 util/kxmessages.cpp
 util/kundostack.cpp
 widgets/kactionselector.cpp
 widgets/kanimatedbutton.cpp
 widgets/karrowbutton.cpp
 widgets/kdialogbuttonbox.cpp
 widgets/kbuttongroup.cpp
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
 widgets/khbox.cpp
 widgets/khelpmenu.cpp
 widgets/khistorycombobox.cpp
 widgets/kkeysequencewidget.cpp
 widgets/kinformationlabel.cpp
 widgets/kled.cpp
 widgets/klineedit.cpp
 widgets/kmainwindow.cpp
 widgets/kmenu.cpp
 widgets/kmenubar.cpp
 widgets/knuminput.cpp
 widgets/kpixmapregionselectorwidget.cpp
 widgets/kpushbutton.cpp
 widgets/krestrictedline.cpp
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
 widgets/ktoolbar.cpp
 widgets/kurllabel.cpp
 widgets/kvbox.cpp
 widgets/kxyselector.cpp
 xmlgui/kxmlguiwindow.cpp
 xmlgui/kmainwindowiface.cpp
 xmlgui/kmenumenuhandler_p.cpp
 xmlgui/ktoolbarhandler.cpp
 xmlgui/kxmlguiclient.cpp
 xmlgui/kxmlguifactory.cpp
 xmlgui/kxmlguifactory_p.cpp
 xmlgui/kxmlguibuilder.cpp
)

if (Q_WS_X11)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} shortcuts/kglobalaccel_x11.cpp util/kkeyserver_x11.cpp )
endif (Q_WS_X11)
if (Q_WS_MAC)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} shortcuts/kglobalaccel_mac.cpp util/kkeyserver_mac.cpp )
endif (Q_WS_MAC)
if (Q_WS_WIN)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} shortcuts/kglobalaccel_win.cpp kernel/kapplication_win.cpp )
endif (Q_WS_WIN)

if (Q_WS_X11)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} windowmanagement/kwm_x11.cpp
                                         windowmanagement/kwindowinfo_x11.cpp
                                         windowmanagement/netwm.cpp
       )
endif (Q_WS_X11)
if (Q_WS_MAC)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} windowmanagement/kwm_mac.cpp
       )     
endif (Q_WS_MAC)
if (Q_WS_WIN)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} windowmanagement/kwm_win.cpp)
endif (Q_WS_WIN)

kde4_automoc(${kdeui_LIB_SRCS} dialogs/kdialogqueue_p.h)

set( kdeui_UI
   dialogs/kpassworddialog.ui
   dialogs/knewpassworddialog.ui
   dialogs/kshortcutsdialog.ui
   widgets/kshortcutwidget.ui
   sonnet/kspell2ui.ui 
   sonnet/configui.ui
   )

kde4_add_ui_files(kdeui_LIB_SRCS ${kdeui_UI} )

qt4_add_dbus_interfaces(kdeui_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/jobs/org.kde.UiServer.xml )
qt4_add_dbus_adaptor(kdeui_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/jobs/org.kde.UiServerCallbacks.xml kuiserverjobtracker_p.h KSharedUiServerProxy uiservercallbacks )

kde4_add_library(kdeui SHARED ${kdeui_LIB_SRCS})

target_link_libraries(kdeui  ${QT_QTSVG_LIBRARY} ${KDE4_KDECORE_LIBS} ${X11_LIBRARIES} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} kdefx )

if(X11_XTest_FOUND)
   target_link_libraries(kdeui ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

set_target_properties(kdeui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kdeui  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES dialogs/aboutkde.png dialogs/ktip-bulb.png dialogs/ktip-background.png  DESTINATION  ${DATA_INSTALL_DIR}/kdeui/pics )
install( FILES ui_standards.rc  DESTINATION  ${CONFIG_INSTALL_DIR}/ui )
install( FILES
 kdeui_export.h
 actions/kactioncollection.h
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
 colors/kcolorbutton.h
 colors/kcolorchoosermode.h
 colors/kcolorcombo.h
 colors/kcolordialog.h
 colors/kcolormimedata.h
 colors/kcolorvalueselector.h
 colors/khuesaturationselect.h
 config/kconfigskeleton.h
 dialogs/kaboutapplicationdialog.h
 dialogs/kaboutkdedialog.h
 dialogs/kassistantdialog.h
 dialogs/kbugreport.h
 dialogs/kconfigdialog.h
 dialogs/kconfigdialogmanager.h
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
 fonts/kfontdialog.h
 fonts/kfontrequester.h
 icons/kiconeffect.h
 icons/kicon.h
 icons/kiconloader.h
 icons/kicontheme.h
 itemviews/klistwidget.h
 itemviews/klistwidgetsearchline.h
 itemviews/ktreewidgetsearchline.h
 jobs/kdialogjobuidelegate.h
 jobs/kabstractwidgetjobtracker.h
 jobs/kwidgetjobtracker.h
 jobs/kstatusbarjobtracker.h
 jobs/kuiserverjobtracker.h
 kernel/kapplication.h
 kernel/kuniqueapplication.h
 kernel/ksessionmanager.h
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
 shortcuts/kacceleratormanager.h
 shortcuts/kgesture.h
 util/fixx11h.h
 util/kaccelgen.h
 util/kcompletion.h
 util/kcrash.h
 util/kcursor.h
 util/kguiitem.h
 util/kkeyserver.h
 util/kkeyserver_x11.h
 util/kmanagerselection.h
 util/knotification.h
 util/knotificationrestrictions.h
 util/knumvalidator.h
 util/kpalette.h
 util/kpassivepopup.h
 util/kpassivepopupmessagehandler.h
 util/kpixmapprovider.h
 util/kstandardguiitem.h
 util/ksvgrenderer.h
 util/ksystemtrayicon.h
 util/kwordwrap.h
 util/kxerrorhandler.h
 util/kxmessages.h
 util/kundostack.h
 widgets/kactionselector.h
 widgets/kanimatedbutton.h
 widgets/karrowbutton.h
 widgets/kdatepicker.h
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
 widgets/khbox.h
 widgets/khelpmenu.h
 widgets/khistorycombobox.h
 widgets/kkeysequencewidget.h
 widgets/kinformationlabel.h
 widgets/kled.h
 widgets/klineedit.h
 widgets/kmainwindow.h
 widgets/kmenu.h
 widgets/kmenubar.h
 widgets/knuminput.h
 widgets/kpixmapregionselectorwidget.h
 widgets/kpushbutton.h
 widgets/krestrictedline.h
 widgets/kseparator.h
 widgets/ksqueezedtextlabel.h
 widgets/kstringvalidator.h
 widgets/ktextbrowser.h
 widgets/ktextedit.h
 widgets/ktimezonewidget.h
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
 windowmanagement/kwm.h
 windowmanagement/kwindowinfo.h
 windowmanagement/netwm_def.h
 windowmanagement/netwm.h
 xmlgui/kxmlguiwindow.h
 xmlgui/kxmlguiclient.h
 xmlgui/kxmlguibuilder.h
 xmlgui/kxmlguifactory.h
 DESTINATION ${INCLUDE_INSTALL_DIR})

install( FILES 
  sonnet/dialog.h 
  sonnet/highlighter.h 
  sonnet/configdialog.h   
  sonnet/configwidget.h  
  DESTINATION  ${INCLUDE_INSTALL_DIR}/sonnet )

install(PROGRAMS preparetips DESTINATION ${BIN_INSTALL_DIR} )

install( FILES colors/40.colors colors/Web.colors colors/Royal.colors colors/Rainbow.colors DESTINATION ${CONFIG_INSTALL_DIR}/colors)

install(FILES jobs/org.kde.UiServer.xml DESTINATION ${DBUS_INTERFACES_DIR})
install(FILES jobs/org.kde.UiServerCallbacks.xml DESTINATION ${DBUS_INTERFACES_DIR})


