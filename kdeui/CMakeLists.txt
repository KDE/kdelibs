
project(kdeui)

KDE4_NO_ENABLE_FINAL(kdeui)

include_directories(
 ${CMAKE_SOURCE_DIR}/kdefx
 ${CMAKE_SOURCE_DIR}/interfaces
 ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}/actions
 ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
 ${CMAKE_CURRENT_SOURCE_DIR}/findreplace
 ${CMAKE_CURRENT_SOURCE_DIR}/icons
 ${CMAKE_CURRENT_SOURCE_DIR}/itemviews
 ${CMAKE_CURRENT_SOURCE_DIR}/jobs
 ${CMAKE_CURRENT_SOURCE_DIR}/kernel
 ${CMAKE_CURRENT_SOURCE_DIR}/paged
 ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts
 ${CMAKE_CURRENT_SOURCE_DIR}/widgets
 ${CMAKE_CURRENT_SOURCE_DIR}/util
 ${CMAKE_CURRENT_SOURCE_DIR}/windowmanagement
 ${CMAKE_CURRENT_SOURCE_DIR}/xmlgui
 ${CMAKE_CURRENT_SOURCE_DIR}/liveui
)

add_subdirectory( about )
add_subdirectory( tests )
add_subdirectory( liveui/tests )

########### next target ###############

set(kdeui_LIB_SRCS
 klanguagebutton.cpp
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
 dialogs/kaboutkdedialog.cpp
 dialogs/kassistantdialog.cpp
 dialogs/kconfigdialog.cpp
 dialogs/kconfigdialogmanager.cpp
 dialogs/kbugreport.cpp
 dialogs/kdialog.cpp
 dialogs/kedittoolbar.cpp
 dialogs/kcolordialog.cpp
 dialogs/kfontdialog.cpp
 dialogs/kinputdialog.cpp
 dialogs/kkeydialog.cpp
 dialogs/kmessagebox.cpp
 dialogs/kmessageboxmessagehandler.cpp
 dialogs/kaboutapplicationdialog.cpp
 dialogs/knewpassworddialog.cpp
 dialogs/kpassworddialog.cpp
 dialogs/kpixmapregionselectordialog.cpp
 dialogs/kprogressdialog.cpp
 dialogs/kshortcutdialog.cpp
 dialogs/ksplashscreen.cpp
 dialogs/ktip.cpp
 findreplace/kfind.cpp
 findreplace/kreplace.cpp
 findreplace/kfinddialog.cpp
 findreplace/kreplacedialog.cpp
 icons/kiconeffect.cpp
 icons/kiconengine.cpp
 icons/kicon.cpp
 icons/kiconloader.cpp
 icons/kicontheme.cpp
 itemviews/k3iconview.cpp
 itemviews/k3listview.cpp
 itemviews/klistwidget.cpp
 itemviews/klistwidgetsearchline.cpp
 itemviews/ktreewidgetsearchline.cpp
 jobs/kabstractwidgetjobtracker.cpp
 jobs/kwidgetjobtracker.cpp
 jobs/kstatusbarjobtracker.cpp
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
 shortcuts/kshortcut.cpp
 shortcuts/kstandardshortcut.cpp
 util/kacceleratormanager.cpp
 util/kcheckaccelerators.cpp
 util/kckey.cpp
 util/kcolormimedata.cpp
 util/kcommand.cpp
 util/kcompletion.cpp
 util/kcompletionbase.cpp
 util/kconfiggroupgui.cpp
 util/kcrash.cpp
 util/kcursor.cpp
 util/kglobalaccel.cpp
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
 widgets/kbuttonbox.cpp
 widgets/kdialogbuttonbox.cpp
 widgets/kbuttongroup.cpp
 widgets/kcharselect.cpp
 widgets/kcmodule.cpp
 widgets/kcolorbutton.cpp
 widgets/kcolorcombo.cpp
 widgets/kcolorvalueselector.cpp
 widgets/kcombobox.cpp
 widgets/kcompletionbox.cpp
 widgets/kdatepicker.cpp
 widgets/kdatetable.cpp
 widgets/kdatetimewidget.cpp
 widgets/kdatewidget.cpp
 widgets/keditlistbox.cpp
 widgets/kfontrequester.cpp
 widgets/khbox.cpp
 widgets/khelpmenu.cpp
 widgets/kkeybutton.cpp
 widgets/kinformationlabel.cpp
 widgets/kled.cpp
 widgets/klineedit.cpp
 widgets/kmenu.cpp
 widgets/kmenubar.cpp
 widgets/knuminput.cpp
 widgets/kpixmapregionselectorwidget.cpp
 widgets/kpushbutton.cpp
 widgets/krestrictedline.cpp
 widgets/kruler.cpp
 widgets/kselector.cpp
 widgets/kseparator.cpp
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
 xmlgui/kmainwindow.cpp
 xmlgui/kmainwindowiface.cpp
 xmlgui/kmenumenuhandler_p.cpp
 xmlgui/ktoolbarhandler.cpp
 xmlgui/kxmlguiclient.cpp
 xmlgui/kxmlguifactory.cpp
 xmlgui/kxmlguifactory_p.cpp
 xmlgui/kxmlguibuilder.cpp
 liveui/kliveui.cpp
)

if (Q_WS_X11)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kglobalaccel_x11.cpp util/kkeyserver_x11.cpp )
endif (Q_WS_X11)
if (Q_WS_MAC)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kglobalaccel_mac.cpp util/kkeyserver_mac.cpp )
endif (Q_WS_MAC)
if (Q_WS_WIN)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kglobalaccel_win.cpp kernel/kapplication_win.cpp )
endif (Q_WS_WIN)

if (UNIX)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} windowmanagement/kwin.cpp
                                         windowmanagement/kwinmodule.cpp
                                         windowmanagement/netwm.cpp
       )
endif (UNIX)


kde4_automoc(${kdeui_LIB_SRCS})

set( kdeui_UI
   dialogs/kkeydialog.ui
   dialogs/kshortcutdialog_simple.ui
   dialogs/kshortcutdialog_advanced.ui
   dialogs/kpassworddialog.ui
   dialogs/knewpassworddialog.ui
   )

kde4_add_ui_files(kdeui_LIB_SRCS ${kdeui_UI} )

kde4_add_library(kdeui SHARED ${kdeui_LIB_SRCS})

target_link_libraries(kdeui  ${QT_QTSVG_LIBRARY} ${KDE4_KDECORE_LIBS} ${X11_LIBRARIES} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} kdefx )

set_target_properties(kdeui PROPERTIES VERSION 5.0.0 SOVERSION 5 )
install(TARGETS kdeui  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES dialogs/aboutkde.png dialogs/ktip-bulb.png dialogs/ktip-background.png  DESTINATION  ${DATA_INSTALL_DIR}/kdeui/pics )
install( FILES ui_standards.rc  DESTINATION  ${CONFIG_INSTALL_DIR}/ui )
install( FILES
 klanguagebutton.h
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
 dialogs/kaboutapplicationdialog.h
 dialogs/kaboutkdedialog.h
 dialogs/kassistantdialog.h
 dialogs/kbugreport.h
 dialogs/kcolordialog.h
 dialogs/kconfigdialog.h
 dialogs/kconfigdialogmanager.h
 dialogs/kdialog.h
 dialogs/kedittoolbar.h
 dialogs/kfontdialog.h
 dialogs/kinputdialog.h
 dialogs/kkeydialog.h
 dialogs/kmessagebox.h
 dialogs/kmessageboxmessagehandler.h
 dialogs/knewpassworddialog.h
 dialogs/kpassworddialog.h
 dialogs/kpixmapregionselectordialog.h
 dialogs/kprogressdialog.h
 dialogs/kshortcutdialog.h
 dialogs/ksplashscreen.h
 dialogs/ktip.h
 findreplace/kfind.h
 findreplace/kreplace.h
 findreplace/kfinddialog.h
 findreplace/kreplacedialog.h
 icons/kiconeffect.h
 icons/kiconengine.h
 icons/kicon.h
 icons/kiconloader.h
 icons/kicontheme.h
 itemviews/k3iconview.h
 itemviews/k3listview.h
 itemviews/k3listviewlineedit.h
 itemviews/klistwidget.h
 itemviews/klistwidgetsearchline.h
 itemviews/ktreewidgetsearchline.h
 jobs/kabstractwidgetjobtracker.h
 jobs/kwidgetjobtracker.h
 jobs/kstatusbarjobtracker.h
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
 shortcuts/kshortcut.h
 shortcuts/kstandardshortcut.h
 util/fixx11h.h
 util/kacceleratormanager.h
 util/kaccelgen.h
 util/kcolormimedata.h
 util/kcommand.h
 util/kcompletion.h
 util/kcrash.h
 util/kcursor.h
 util/kglobalaccel.h
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
 widgets/kfontrequester.h
 widgets/kcmodule.h
 widgets/kcolorbutton.h
 widgets/kcolorcombo.h
 widgets/kcompletionbox.h
 widgets/kcombobox.h
 widgets/kbuttonbox.h
 widgets/kbuttongroup.h
 widgets/kcharselect.h
 widgets/kcolorvalueselector.h
 widgets/kdatetable.h
 widgets/kdatetimewidget.h
 widgets/kdatewidget.h
 widgets/kdialogbuttonbox.h
 widgets/keditlistbox.h
 widgets/khbox.h
 widgets/khelpmenu.h
 widgets/kkeybutton.h
 widgets/kinformationlabel.h
 widgets/kled.h
 widgets/klineedit.h
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
 widgets/kstatusbar.h
 widgets/ktoolbar.h
 widgets/kurllabel.h
 widgets/kvbox.h
 widgets/kxyselector.h
 windowmanagement/kwin.h
 windowmanagement/kwinmodule.h
 windowmanagement/netwm_def.h
 windowmanagement/netwm.h
 xmlgui/kmainwindow.h
 xmlgui/kxmlguiclient.h
 xmlgui/kxmlguibuilder.h
 xmlgui/kxmlguifactory.h
 liveui/kliveui.h
 DESTINATION ${INCLUDE_INSTALL_DIR})

install(PROGRAMS preparetips DESTINATION ${BIN_INSTALL_DIR} )

install( FILES dialogs/40.colors dialogs/Web.colors dialogs/Royal.colors dialogs/Rainbow.colors DESTINATION ${CONFIG_INSTALL_DIR}/colors)



