kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


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

target_link_libraries(editor  ${QT_AND_KDECORE_LIBS} kjsembed )

install_targets(/bin editor )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#bin_PROGRAMS = editor 
#
#editor_SOURCES =  bind_qaction.cpp bind_qdialog.cpp bind_qicon.cpp \
#                  bind_qlcdnumber.cpp bind_qtextcursor.cpp bind_qtextedit.cpp \
#                  bind_qtimer.cpp bind_qwidget.cpp bind_qworkspace.cpp \
#                  main.cpp mainwindow.cpp mdichild.cpp numberedtextview.cpp \
#                  mdi.qrc
#editor_METASOURCES = AUTO 
#editor_LDFLAGS = $(all_libraries) 
#editor_LDADD = -lkjsembed
#
