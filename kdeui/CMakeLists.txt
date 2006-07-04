
project(kdeui)

include_directories(
 ${CMAKE_SOURCE_DIR}/kdefx
 ${CMAKE_SOURCE_DIR}/interfaces
 ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}/findreplace
)

remove_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)

if (Q_WS_X11)
   add_subdirectory( kdetrayproxy )
endif (Q_WS_X11)

add_subdirectory( findreplace )
add_subdirectory( tests )
add_subdirectory( liveui/tests )
add_subdirectory( about )


########### next target ###############

set(kdeui_LIB_SRCS
 kmenu.cpp
 ktoolbar.cpp
 kaction.cpp
 kstdaction.cpp
 kactioncollection.cpp
 ktoggleaction.cpp
 kselectaction.cpp
 krecentfilesaction.cpp
 kfontaction.cpp
 kfontsizeaction.cpp
 kactionmenu.cpp
 ktoolbarpopupaction.cpp
 ktoggletoolbaraction.cpp
 ktogglefullscreenaction.cpp
 k3widgetaction.cpp
 kpastetextaction.cpp
 kseparatoraction.cpp
 kurllabel.cpp
 kmenubar.cpp
 kinputdialog.cpp
 knuminput.cpp
 klineedit.cpp
 k3listview.cpp
 knotificationrestrictions.cpp
 kprogressdialog.cpp
 kprogressbar.cpp
 kcolordialog.cpp
 kselector.cpp
 kdatepicker.cpp
 kdatetable.cpp
 kfontrequester.cpp
 kfontdialog.cpp
 ktabctl.cpp
 kstatusbar.cpp
 kmainwindow.cpp
 keditlistbox.cpp
 kseparator.cpp
 krestrictedline.cpp
 kcolorbutton.cpp
 ksystemtray.cpp
 kbuttonbox.cpp
 keditcl1.cpp
 keditcl2.cpp
 kled.cpp
 kshortcutdialog.cpp
 kkeybutton.cpp
 kkeydialog.cpp
 kruler.cpp
 kcursor.cpp
 kcharselect.cpp
 kcolormimedata.cpp
 knumvalidator.cpp
 kdialog.cpp
 kpagedialog.cpp
 kpagemodel.cpp
 kpageview.cpp
 kpageview_p.cpp
 kpagewidget.cpp
 kpagewidgetmodel.cpp
 kaboutdialog.cpp
 kauthicon.cpp
 kmessagebox.cpp
 kdualcolorbutton.cpp
 ktextbrowser.cpp
 khelpmenu.cpp
 k3iconview.cpp
 klistbox.cpp
 kwordwrap.cpp
 kbugreport.cpp
 kcombobox.cpp
 kpassworddialog.cpp
 kxmlguiclient.cpp
 kxmlguifactory.cpp
 kxmlguifactory_p.cpp
 kxmlguibuilder.cpp
 kmenumenuhandler_p.cpp
 kedittoolbar.cpp
 kanimwidget.cpp
 kaboutkde.cpp
 kaboutapplication.cpp
 kcolorcombo.cpp
 kpushbutton.cpp
 kcompletionbox.cpp
 ksqueezedtextlabel.cpp
 kcmodule.cpp
 kcommand.cpp
 kwindowlistmenu.cpp
 ktip.cpp
 kdatewidget.cpp
 karrowbutton.cpp
 kmainwindowiface.cpp
 kguiitem.cpp
 kstdguiitem.cpp
 kstringvalidator.cpp
 kactivelabel.cpp
 ktextedit.cpp
 kpassivepopup.cpp
 ktoolbarhandler.cpp
 kdatetimewidget.cpp
 ktimezonewidget.cpp
 ktabbar.cpp
 ktabwidget.cpp
 ksplashscreen.cpp
 kconfigdialog.cpp
 kactionselector.cpp
 klanguagebutton.cpp
 k3listviewsearchline.cpp
 kpixmapregionselectorwidget.cpp
 kpixmapregionselectordialog.cpp
 kwhatsthismanager.cpp
 k3iconviewsearchline.cpp
 ktoolbarlabelaction.cpp
 khbox.cpp
 kvbox.cpp
 kconfigdialogmanager.cpp
 knotification.cpp
 knotificationmanager.cpp
 kiconengine.cpp
 kicon.cpp
 ktoolbarspaceraction.cpp
 kmanagerselection.cpp
 kglobalaccel.cpp
 ktreewidgetsearchline.cpp
 kkeyserver.cpp
 kcompletionbase.cpp
 kcompletion.cpp
 kmessageboxmessagehandler.cpp
 kpassivepopupmessagehandler.cpp
 kbuttongroup.cpp
 findreplace/kfind.cpp
 findreplace/kreplace.cpp
 findreplace/kfinddialog.cpp
 findreplace/kreplacedialog.cpp
 liveui/kliveui.cpp
)

if (Q_WS_X11)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} kglobalaccel_x11.cpp kkeyserver_x11.cpp )
endif (Q_WS_X11)
if (Q_WS_MAC)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} kglobalaccel_mac.cpp kkeyserver_mac.cpp )
endif (Q_WS_MAC)
if (Q_WS_WIN)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} kglobalaccel_win.cpp )
endif (Q_WS_WIN)


SET_SOURCE_FILES_PROPERTIES(liveui/kliveui.cpp PROPERTIES SKIP_AUTOMOC TRUE)

kde4_automoc(${kdeui_LIB_SRCS})

set( kdeui_UI
   kkeydialog.ui
   kshortcutdialog_simple.ui
   kshortcutdialog_advanced.ui
   )

kde4_add_ui_files(kdeui_LIB_SRCS ${kdeui_UI} )

kde4_add_library(kdeui SHARED ${kdeui_LIB_SRCS})

target_link_libraries(kdeui  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} kdefx )

set_target_properties(kdeui PROPERTIES VERSION 5.0.0 SOVERSION 5 )
install(TARGETS kdeui  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES aboutkde.png ktip-bulb.png ktip-background.png  DESTINATION  ${DATA_INSTALL_DIR}/kdeui/pics )
install( FILES ui_standards.rc  DESTINATION  ${CONFIG_INSTALL_DIR}/ui )
install( FILES
 kprogressbar.h
 kprogressdialog.h
 kcolordialog.h
 kselector.h
 kdatepicker.h
 kdatetable.h
 kfontdialog.h
 kmenu.h
 kfontrequester.h
 ktabctl.h
 kstatusbar.h
 kmainwindow.h
 ktoolbar.h
 kmenubar.h
 knuminput.h
 knotificationrestrictions.h
 kseparator.h
 klineedit.h
 krestrictedline.h
 kcolorbutton.h
 ksystemtray.h
 kbuttonbox.h
 keditcl.h
 kled.h
 keditlistbox.h
 kkeydialog.h
 kkeybutton.h
 kurllabel.h
 kruler.h
 kcursor.h
 kinputdialog.h
 kactivelabel.h
 kcharselect.h
 kcolormimedata.h
 knumvalidator.h
 kdialog.h
 kpagedialog.h
 kpagemodel.h
 kpageview.h
 kpagewidget.h
 kpagewidgetmodel.h
 kaboutdialog.h
 kauthicon.h
 kmessagebox.h
 kdualcolorbutton.h
 ktextbrowser.h
 kaction.h
 kactioncollection.h
 kactionclasses.h
 kseparatoraction.h
 ktoggleaction.h
 kselectaction.h
 krecentfilesaction.h
 kfontaction.h
 kfontsizeaction.h
 kactionmenu.h
 ktoolbarpopupaction.h
 ktoggletoolbaraction.h
 ktogglefullscreenaction.h
 k3widgetaction.h
 kpastetextaction.h
 khelpmenu.h
 kcmodule.h
 kstdaction.h
 kcombobox.h
 k3iconview.h
 k3listview.h
 klistbox.h
 kbugreport.h
 kpassworddialog.h
 kxmlguiclient.h
 kxmlguibuilder.h
 kxmlguifactory.h
 kwordwrap.h
 kedittoolbar.h
 kanimwidget.h
 kaboutkde.h
 kaboutapplication.h
 kpushbutton.h
 kcolorcombo.h
 kcompletionbox.h
 ksqueezedtextlabel.h
 kcommand.h
 kwindowlistmenu.h
 kaccelgen.h
 ktip.h
 kdatewidget.h
 karrowbutton.h
 kguiitem.h
 kstdguiitem.h
 kstringvalidator.h
 ktextedit.h
 kpassivepopup.h
 kdatetimewidget.h
 ktimezonewidget.h
 ktabbar.h
 ktabwidget.h
 ksplashscreen.h
 kconfigdialog.h
 kactionselector.h
 klanguagebutton.h
 k3listviewsearchline.h
 kpixmapregionselectorwidget.h
 kpixmapregionselectordialog.h
 k3iconviewsearchline.h
 ktoolbarlabelaction.h
 kshortcutdialog.h
 khbox.h
 kvbox.h
 kconfigdialogmanager.h
 knotification.h
 kiconengine.h
 kicon.h
 ktoolbarspaceraction.h
 kmanagerselection.h
 kglobalaccel.h
 ktreewidgetsearchline.h
 kkeyserver_x11.h
 kkeyserver.h
 kcompletion.h
 kmessageboxmessagehandler.h
 kpassivepopupmessagehandler.h
 kbuttongroup.h
 DESTINATION ${INCLUDE_INSTALL_DIR})

QT4_GENERATE_MOC(liveui/kliveui.cpp ${CMAKE_CURRENT_BINARY_DIR}/kliveui.moc)
MACRO_ADD_FILE_DEPENDENCIES(liveui/kliveui.cpp ${CMAKE_CURRENT_BINARY_DIR}/kliveui.moc)

install(PROGRAMS preparetips DESTINATION ${BIN_INSTALL_DIR} )




