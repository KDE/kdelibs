


########### next target ###############

set(editor_SRCS
bind_qaction.cpp
bind_qdialog.cpp
bind_qicon.cpp
bind_qlcdnumber.cpp
bind_qtextcursor.cpp
bind_qtextedit.cpp
bind_qtimer.cpp
bind_qwidget.cpp
bind_qworkspace.cpp
main.cpp
mainwindow.cpp
mdichild.cpp
numberedtextview.cpp
)

kde4_automoc(${editor_SRCS})

set( editor_QRC
mdi.qrc
)

qt4_add_resources(editor_SRCS ${editor_QRC} )

kde4_add_executable(editor ${editor_SRCS})

target_link_libraries(editor  ${KDE4_KDECORE_LIBS} kjsembed )

install(TARGETS editor DESTINATION bin)


########### install files ###############





