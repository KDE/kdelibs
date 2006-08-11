
project(kdeui)

include_directories(
 ${CMAKE_SOURCE_DIR}/kdefx
 ${CMAKE_SOURCE_DIR}/interfaces
 ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}/findreplace
 ${CMAKE_CURRENT_SOURCE_DIR}/widgets
 ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
 ${CMAKE_CURRENT_SOURCE_DIR}/util
 ${CMAKE_CURRENT_SOURCE_DIR}/itemviews
)

add_subdirectory( findreplace )
add_subdirectory( widgets )
add_subdirectory( dialogs )
add_subdirectory( util )
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
 kdatetable.cpp
 ktabctl.cpp
 kstatusbar.cpp
 kmainwindow.cpp
 kseparator.cpp
 krestrictedline.cpp
 kruler.cpp
 kpagemodel.cpp
 kpageview.cpp
 kpageview_p.cpp
 kpagewidget.cpp
 kpagewidgetmodel.cpp
 kmessagebox.cpp
 ktextbrowser.cpp
 khelpmenu.cpp
 dialogs/kbugreport.cpp
 kxmlguiclient.cpp
 kxmlguifactory.cpp
 kxmlguifactory_p.cpp
 kxmlguibuilder.cpp
 kmenumenuhandler_p.cpp
 kedittoolbar.cpp
 kanimwidget.cpp
 kcmodule.cpp
 kwindowlistmenu.cpp
 ktip.cpp
 karrowbutton.cpp
 kmainwindowiface.cpp
 kguiitem.cpp
 kstdguiitem.cpp
 kactivelabel.cpp
 ktoolbarhandler.cpp
 ksplashscreen.cpp
 kactionselector.cpp
 klanguagebutton.cpp
 kwhatsthismanager.cpp
 kpixmapregionselectorwidget.cpp
 ktoolbarlabelaction.cpp
 ktoolbarspaceraction.cpp
 kmessageboxmessagehandler.cpp
 dialogs/kwizard.cpp
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
 util/kcolormimedata.cpp
 util/kcommand.cpp
 util/kcompletion.cpp
 util/kcompletionbase.cpp
 util/kcursor.cpp
 util/kglobalaccel.cpp
 util/kicon.cpp
 util/kiconengine.cpp
 util/kkeyserver.cpp
 util/kmanagerselection.cpp
 util/knotification.cpp
 util/knotificationmanager.cpp
 util/knotificationrestrictions.cpp
 util/knumvalidator.cpp
 util/kpassivepopup.cpp
 util/kpassivepopupmessagehandler.cpp
 util/kstringvalidator.cpp
 util/ksystemtrayicon.cpp
 util/kwordwrap.cpp
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
 dialogs/kaboutdialog.cpp
 dialogs/kdialog.cpp
 dialogs/kcolordialog.cpp
 dialogs/kfontdialog.cpp
 dialogs/kinputdialog.cpp
 dialogs/kkeydialog.cpp
 dialogs/kpagedialog.cpp
 dialogs/kaboutapplication.cpp
 dialogs/kpassworddialog.cpp
 dialogs/kprogressdialog.cpp
 dialogs/kshortcutdialog.cpp
 dialogs/kconfigdialogmanager.cpp
 dialogs/kaboutkde.cpp
 dialogs/kconfigdialog.cpp
 dialogs/kpixmapregionselectordialog.cpp
)

if (Q_WS_X11)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kglobalaccel_x11.cpp util/kkeyserver_x11.cpp )
endif (Q_WS_X11)
if (Q_WS_MAC)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kglobalaccel_mac.cpp util/kkeyserver_mac.cpp )
endif (Q_WS_MAC)
if (Q_WS_WIN)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kglobalaccel_win.cpp )
endif (Q_WS_WIN)


SET_SOURCE_FILES_PROPERTIES(liveui/kliveui.cpp PROPERTIES SKIP_AUTOMOC TRUE)

kde4_automoc(${kdeui_LIB_SRCS})

set( kdeui_UI
   dialogs/kkeydialog.ui
   dialogs/kshortcutdialog_simple.ui
   dialogs/kshortcutdialog_advanced.ui
   )

kde4_add_ui_files(kdeui_LIB_SRCS ${kdeui_UI} )

kde4_add_library(kdeui SHARED ${kdeui_LIB_SRCS})

target_link_libraries(kdeui  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} kdefx )

set_target_properties(kdeui PROPERTIES VERSION 5.0.0 SOVERSION 5 )
install(TARGETS kdeui  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES dialogs/aboutkde.png ktip-bulb.png ktip-background.png  DESTINATION  ${DATA_INSTALL_DIR}/kdeui/pics )
install( FILES ui_standards.rc  DESTINATION  ${CONFIG_INSTALL_DIR}/ui )
install( FILES
 kdatetable.h
 ktabctl.h
 kstatusbar.h
 kmainwindow.h
 ktoolbar.h
 kmenubar.h
 kseparator.h
 krestrictedline.h
 kurllabel.h
 kruler.h
 kactivelabel.h
 kpagemodel.h
 kpageview.h
 kpagewidget.h
 kpagewidgetmodel.h
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
 kxmlguiclient.h
 kxmlguibuilder.h
 kxmlguifactory.h
 kedittoolbar.h
 kanimwidget.h
 kwindowlistmenu.h
 ktip.h
 karrowbutton.h
 kguiitem.h
 kstdguiitem.h
 ksplashscreen.h
 kactionselector.h
 klanguagebutton.h
 kpixmapregionselectorwidget.h
 ktoolbarlabelaction.h
 ktoolbarspaceraction.h
 kmessageboxmessagehandler.h
 DESTINATION ${INCLUDE_INSTALL_DIR})

QT4_GENERATE_MOC(liveui/kliveui.cpp ${CMAKE_CURRENT_BINARY_DIR}/kliveui.moc)
MACRO_ADD_FILE_DEPENDENCIES(liveui/kliveui.cpp ${CMAKE_CURRENT_BINARY_DIR}/kliveui.moc)

install(PROGRAMS preparetips DESTINATION ${BIN_INSTALL_DIR} )




