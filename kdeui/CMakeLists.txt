
project(kdeui)

KDE4_NO_ENABLE_FINAL(kdeui)

include_directories(
 ${CMAKE_SOURCE_DIR}/kdefx
 ${CMAKE_SOURCE_DIR}/interfaces
 ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}/findreplace
 ${CMAKE_CURRENT_SOURCE_DIR}/widgets
 ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
 ${CMAKE_CURRENT_SOURCE_DIR}/util
 ${CMAKE_CURRENT_SOURCE_DIR}/kernel
 ${CMAKE_CURRENT_SOURCE_DIR}/itemviews
)

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
 kcodecaction.cpp
 kactionmenu.cpp
 ktoolbarpopupaction.cpp
 ktoggletoolbaraction.cpp
 ktogglefullscreenaction.cpp
 kpastetextaction.cpp
 kseparatoraction.cpp
 kurllabel.cpp
 kmenubar.cpp
 kdatetable.cpp
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
 ktextbrowser.cpp
 khelpmenu.cpp
 kxmlguiclient.cpp
 kxmlguifactory.cpp
 kxmlguifactory_p.cpp
 kxmlguibuilder.cpp
 kmenumenuhandler_p.cpp
 kanimwidget.cpp
 kcmodule.cpp
 kwindowlistmenu.cpp
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
 findreplace/kfind.cpp
 findreplace/kreplace.cpp
 findreplace/kfinddialog.cpp
 findreplace/kreplacedialog.cpp
 itemviews/k3iconview.cpp
 itemviews/k3iconviewsearchline.cpp
 itemviews/k3listview.cpp
 itemviews/k3listviewsearchline.cpp
 itemviews/klistbox.cpp
 itemviews/ktreewidgetsearchline.cpp
 kernel/kapplication.cpp
 kernel/kuniqueapplication.cpp
 kernel/kstartupinfo.cpp
 util/kacceleratormanager.cpp
 util/kcheckaccelerators.cpp
 util/kcolormimedata.cpp
 util/kcommand.cpp
 util/kcompletion.cpp
 util/kcompletionbase.cpp
 util/kcrash.cpp
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
 util/kpixmapprovider.cpp
 util/kstringvalidator.cpp
 util/ksystemtrayicon.cpp
 util/kwordwrap.cpp
 util/kxmessages.cpp
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
 dialogs/kassistantdialog.cpp
 dialogs/kbugreport.cpp
 dialogs/kdialog.cpp
 dialogs/kedittoolbar.cpp
 dialogs/kcolordialog.cpp
 dialogs/kfontdialog.cpp
 dialogs/kinputdialog.cpp
 dialogs/kkeydialog.cpp
 dialogs/kmessagebox.cpp
 dialogs/kmessageboxmessagehandler.cpp
 dialogs/kpagedialog.cpp
 dialogs/kaboutapplication.cpp
 dialogs/kpassworddialog.cpp
 dialogs/kprogressdialog.cpp
 dialogs/kshortcutdialog.cpp
 dialogs/ktip.cpp
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
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kglobalaccel_win.cpp kernel/kapplication_win.cpp )
endif (Q_WS_WIN)

if (UNIX)
    set(kdeui_LIB_SRCS ${kdeui_LIB_SRCS} util/kwin.cpp util/kwinmodule.cpp )
endif (UNIX)


kde4_automoc(${kdeui_LIB_SRCS})

set( kdeui_UI
   dialogs/kkeydialog.ui
   dialogs/kshortcutdialog_simple.ui
   dialogs/kshortcutdialog_advanced.ui
   )

kde4_add_ui_files(kdeui_LIB_SRCS ${kdeui_UI} )

kde4_add_library(kdeui SHARED ${kdeui_LIB_SRCS})

target_link_libraries(kdeui  ${KDE4_KDECORE_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} kdefx )

set_target_properties(kdeui PROPERTIES VERSION 5.0.0 SOVERSION 5 )
install(TARGETS kdeui  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES dialogs/aboutkde.png dialogs/ktip-bulb.png dialogs/ktip-background.png  DESTINATION  ${DATA_INSTALL_DIR}/kdeui/pics )
install( FILES ui_standards.rc  DESTINATION  ${CONFIG_INSTALL_DIR}/ui )
install( FILES
 kdatetable.h
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
 kanimwidget.h
 kwindowlistmenu.h
 karrowbutton.h
 kguiitem.h
 kstdguiitem.h
 ksplashscreen.h
 kactionselector.h
 klanguagebutton.h
 kpixmapregionselectorwidget.h
 ktoolbarlabelaction.h
 ktoolbarspaceraction.h
 widgets/kdatepicker.h
 widgets/kmenu.h
 widgets/kfontrequester.h
 widgets/knuminput.h
 widgets/klineedit.h
 widgets/kcolorbutton.h
 widgets/kbuttonbox.h
 widgets/kdialogbuttonbox.h
 widgets/kled.h
 widgets/keditlistbox.h
 widgets/kkeybutton.h
 widgets/kcharselect.h
 widgets/kcombobox.h
 widgets/kpushbutton.h
 widgets/kcolorcombo.h
 widgets/kcompletionbox.h
 widgets/ksqueezedtextlabel.h
 widgets/kdatewidget.h
 widgets/ktextedit.h
 widgets/kdatetimewidget.h
 widgets/ktimezonewidget.h
 widgets/ktabbar.h
 widgets/ktabwidget.h
 widgets/khbox.h
 widgets/kvbox.h
 widgets/kbuttongroup.h
 widgets/kselector.h
 widgets/kcolorvalueselector.h
 widgets/kxyselector.h
 dialogs/kaboutapplication.h
 dialogs/kaboutdialog.h
 dialogs/kaboutkde.h
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
 dialogs/kpagedialog.h
 dialogs/kpassworddialog.h
 dialogs/kpixmapregionselectordialog.h
 dialogs/kprogressdialog.h
 dialogs/kshortcutdialog.h
 dialogs/ktip.h
 findreplace/kfind.h
 findreplace/kreplace.h
 findreplace/kfinddialog.h
 findreplace/kreplacedialog.h
 itemviews/k3iconview.h
 itemviews/k3iconviewsearchline.h
 itemviews/k3listview.h
 itemviews/k3listviewlineedit.h
 itemviews/k3listviewsearchline.h
 itemviews/klistbox.h
 itemviews/ktreewidgetsearchline.h
 kernel/kapplication.h
 kernel/kuniqueapplication.h
 kernel/kstartupinfo.h
 util/kacceleratormanager.h
 util/kaccelgen.h
 util/kcolormimedata.h
 util/kcommand.h
 util/kcompletion.h
 util/kcrash.h
 util/kcursor.h
 util/kglobalaccel.h
 util/kiconengine.h
 util/kicon.h
 util/kkeyserver.h
 util/kkeyserver_x11.h
 util/kmanagerselection.h
 util/knotification.h
 util/knotificationrestrictions.h
 util/knumvalidator.h
 util/kpassivepopup.h
 util/kpassivepopupmessagehandler.h
 util/kpixmapprovider.h
 util/kstringvalidator.h
 util/ksystemtrayicon.h
 util/kwordwrap.h
 util/kxmessages.h
 util/kwinmodule.h
 util/kwin.h
 DESTINATION ${INCLUDE_INSTALL_DIR})

install(PROGRAMS preparetips DESTINATION ${BIN_INSTALL_DIR} )




