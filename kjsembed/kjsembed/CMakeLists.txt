INCLUDE_DIRECTORIES( ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

SET(kjsembed_LIB_SRCS
global.cpp
binding_support.cpp
static_binding.cpp
value_binding.cpp
object_binding.cpp
builtins.cpp
fileio.cpp
jseventmapper.cpp
eventproxy.cpp
slotproxy.cpp
jseventutils.cpp
qobject_binding.cpp
kjsembed.cpp
scalar_binding.cpp
iosupport.cpp
qwidget_binding.cpp
widgetfactory.cpp
qformbuilder_binding.cpp
qpainter_binding.cpp
settings.cpp
svg_binding.cpp
filedialog_binding.cpp
\
)

KDE3_AUTOMOC(${kjsembed_LIB_SRCS})

ADD_LIBRARY(kjsembed SHARED ${kjsembed_LIB_SRCS})

TARGET_LINK_LIBRARIES(kjsembed  ${QT_AND_KDECORE_LIBS} kdeui kparts QtSvg_debug )

SET_TARGET_PROPERTIES(kjsembed PROPERTIES VERSION 4.2.0 SOVERSION 4 )
INSTALL_TARGETS(/lib kjsembed )


########### install files ###############

INSTALL_FILES( /include/kjsembed FILES global.h binding_support.h static_binding.h value_binding.h                            object_binding.h builtins.h fileio.h jseventmapper.h                            eventproxy.h slotproxy.h jseventutils.h qobject_binding.h                            kjsembed.h scalar_binding.h iosupport.h qwidget_binding.h                            widgetfactory.h qformbuilder_binding.h qpainter_binding.h                            settings.h svg_binding.h filedialog_binding.h                            brush.h color.h dom.h font.h image.h pen.h                            pixmap.h point.h rect.h size.h url.h pointer.h )

KDE3_PLACEHOLDER()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#INCLUDES += -DQT_CLEAN_NAMESPACE -DQT_NO_ASCII_CAST
#
#lib_LTLIBRARIES = libkjsembed.la
#
#libkjsembed_la_SOURCES = global.cpp binding_support.cpp static_binding.cpp value_binding.cpp \
#                         object_binding.cpp builtins.cpp fileio.cpp jseventmapper.cpp \
#                         eventproxy.cpp slotproxy.cpp jseventutils.cpp qobject_binding.cpp \
#                         kjsembed.cpp scalar_binding.cpp iosupport.cpp qwidget_binding.cpp \
#                         widgetfactory.cpp qformbuilder_binding.cpp qpainter_binding.cpp \
#                         settings.cpp svg_binding.cpp filedialog_binding.cpp \ 
#                         brush.cpp color.cpp dom.cpp font.cpp image.cpp pen.cpp \
#                         pixmap.cpp point.cpp rect.cpp size.cpp url.cpp 
#
#kjsembedincludedir = $(includedir)/kjsembed
#kjsembedinclude_HEADERS = global.h binding_support.h static_binding.h value_binding.h \
#                          object_binding.h builtins.h fileio.h jseventmapper.h \
#                          eventproxy.h slotproxy.h jseventutils.h qobject_binding.h \
#                          kjsembed.h scalar_binding.h iosupport.h qwidget_binding.h \
#                          widgetfactory.h qformbuilder_binding.h qpainter_binding.h \
#                          settings.h svg_binding.h filedialog_binding.h \
#                          brush.h color.h dom.h font.h image.h pen.h \
#                          pixmap.h point.h rect.h size.h url.h pointer.h
#
#libkjsembed_la_LDFLAGS = -version-info 1:0:0 -no-undefined $(all_libraries) $(KDE_RPATH)
#libkjsembed_la_LIBADD = $(LIB_QT) $(LIB_QTNETWORK) $(LIB_QTDESIGNER) $(LIB_KDECORE) $(LIB_KDEUI) \
#                        $(LIB_KFILE) $(LIB_KHTML) $(LIB_KJS) $(LIBSOCKET) $(LIB_KPARTS) $(LIB_QTXML) \
#			-lQtSvg_debug
#libkjsembed_la_METASOURCES = AUTO
