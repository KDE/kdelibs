
project(kdeui)

include_directories(
 ${CMAKE_SOURCE_DIR}/kdefx
 ${CMAKE_SOURCE_DIR}/interfaces
 ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}/findreplace
 ${CMAKE_CURRENT_SOURCE_DIR}/widgets
 ${CMAKE_CURRENT_SOURCE_DIR}/itemviews
)

add_subdirectory( findreplace )
add_subdirectory( widgets )
add_subdirectory( itemviews )
add_subdirectory( liveui/tests )
add_subdirectory( about )
add_subdirectory( tests )


########### next target ###############

set(kdeui_LIB_SRCS
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
 kpastetextaction.cpp
 kseparatoraction.cpp
 kurllabel.cpp
 kmenubar.cpp
 kinputdialog.cpp
 knotificationrestrictions.cpp
 kprogressdialog.cpp
 kcolordialog.cpp
 kdatetable.cpp
 kfontdialog.cpp
 ktabctl.cpp
 kstatusbar.cpp
 kmainwindow.cpp
 kseparator.cpp
 krestrictedline.cpp
 ksystemtray.cpp
 keditcl1.cpp
 keditcl2.cpp
 kshortcutdialog.cpp
 kkeydialog.cpp
 kruler.cpp
 kcursor.cpp
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
 kmessagebox.cpp
 ktextbrowser.cpp
 khelpmenu.cpp
 kwordwrap.cpp
 kbugreport.cpp
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
 kcmodule.cpp
 kcommand.cpp
 kwindowlistmenu.cpp
 ktip.cpp
 karrowbutton.cpp
 kmainwindowiface.cpp
 kguiitem.cpp
 kstdguiitem.cpp
 kstringvalidator.cpp
 kactivelabel.cpp
 kpassivepopup.cpp
 ktoolbarhandler.cpp
 ksplashscreen.cpp
 kconfigdialog.cpp
 kactionselector.cpp
 klanguagebutton.cpp
 kwhatsthismanager.cpp
 kpixmapregionselectorwidget.cpp
 kpixmapregionselectordialog.cpp
 ktoolbarlabelaction.cpp
 kconfigdialogmanager.cpp
 knotification.cpp
 knotificationmanager.cpp
 kiconengine.cpp
 kicon.cpp
 ktoolbarspaceraction.cpp
 kmanagerselection.cpp
 kglobalaccel.cpp
 kkeyserver.cpp
 kcompletionbase.cpp
 kcompletion.cpp
 kmessageboxmessagehandler.cpp
 kpassivepopupmessagehandler.cpp
 findreplace/kfind.cpp
 findreplace/kreplace.cpp
 findreplace/kfinddialog.cpp
 findreplace/kreplacedialog.cpp
 liveui/kliveui.cpp
 itemviews/k3iconview.cpp
 itemviews/k3iconviewsearchline.cpp
 itemviews/k3listview.cpp
 itemviews/k3listviewsearchline.cpp
 itemviews/klistbox.cpp
 itemviews/ktreewidgetsearchline.cpp
 widgets/kbuttonbox.cpp
 widgets/kdialogbuttonbox.cpp
 widgets/kbuttongroup.cpp
 widgets/kcharselect.cpp
 widgets/kcolorbutton.cpp
 widgets/kcolorcombo.cpp
 widgets/kcolorvalueselector.cpp
 widgets/kcombobox.cpp
 widgets/kcompletionbox.cpp
 widgets/kdatepicker.cpp
 widgets/kdatetimewidget.cpp
 widgets/kdatewidget.cpp
 widgets/keditlistbox.cpp
 widgets/kfontrequester.cpp
 widgets/khbox.cpp
 widgets/kkeybutton.cpp
 widgets/kled.cpp
 widgets/klineedit.cpp
 widgets/kmenu.cpp
 widgets/knuminput.cpp
 widgets/kprogressbar.cpp
 widgets/kpushbutton.cpp
 widgets/kselector.cpp
 widgets/ksqueezedtextlabel.cpp
 widgets/ktabbar.cpp
 widgets/ktabwidget.cpp
 widgets/ktextedit.cpp
 widgets/ktimezonewidget.cpp
 widgets/kvbox.cpp
 widgets/kxyselector.cpp
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
 kprogressdialog.h
 kcolordialog.h
 kdatetable.h
 kfontdialog.h
 ktabctl.h
 kstatusbar.h
 kmainwindow.h
 ktoolbar.h
 kmenubar.h
 knotificationrestrictions.h
 kseparator.h
 krestrictedline.h
 ksystemtray.h
 keditcl.h
 kkeydialog.h
 kurllabel.h
 kruler.h
 kcursor.h
 kinputdialog.h
 kactivelabel.h
 kcolormimedata.h
 knumvalidator.h
 kdialog.h
 kpagedialog.h
 kpagemodel.h
 kpageview.h
 kpagewidget.h
 kpagewidgetmodel.h
 kaboutdialog.h
 kmessagebox.h
 ktextbrowser.h
 kaction.h
 kactioncollection.h
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
 kpastetextaction.h
 khelpmenu.h
 kcmodule.h
 kstdaction.h
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
 kcommand.h
 kwindowlistmenu.h
 kaccelgen.h
 ktip.h
 karrowbutton.h
 kguiitem.h
 kstdguiitem.h
 kstringvalidator.h
 kpassivepopup.h
 ksplashscreen.h
 kconfigdialog.h
 kactionselector.h
 klanguagebutton.h
 kpixmapregionselectorwidget.h
 kpixmapregionselectordialog.h
 ktoolbarlabelaction.h
 kshortcutdialog.h
 kconfigdialogmanager.h
 knotification.h
 kiconengine.h
 kicon.h
 ktoolbarspaceraction.h
 kmanagerselection.h
 kglobalaccel.h
 kkeyserver_x11.h
 kkeyserver.h
 kcompletion.h
 kmessageboxmessagehandler.h
 kpassivepopupmessagehandler.h
 DESTINATION ${INCLUDE_INSTALL_DIR})

QT4_GENERATE_MOC(liveui/kliveui.cpp ${CMAKE_CURRENT_BINARY_DIR}/kliveui.moc)
MACRO_ADD_FILE_DEPENDENCIES(liveui/kliveui.cpp ${CMAKE_CURRENT_BINARY_DIR}/kliveui.moc)

install(PROGRAMS preparetips DESTINATION ${BIN_INSTALL_DIR} )




