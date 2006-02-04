INCLUDE_DIRECTORIES( ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

SET(editor_SRCS
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

KDE3_AUTOMOC(${editor_SRCS})

KDE3_ADD_EXECUTABLE(editor ${editor_SRCS})

TARGET_LINK_LIBRARIES(editor  ${QT_AND_KDECORE_LIBS} kjsembed )

INSTALL_TARGETS(/bin editor )


########### install files ###############


KDE3_PLACEHOLDER()



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
