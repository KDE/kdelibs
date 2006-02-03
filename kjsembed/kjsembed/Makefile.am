INCLUDES = $(all_includes)
INCLUDES += -DQT_CLEAN_NAMESPACE -DQT_NO_ASCII_CAST

lib_LTLIBRARIES = libkjsembed.la

libkjsembed_la_SOURCES = global.cpp binding_support.cpp static_binding.cpp value_binding.cpp \
                         object_binding.cpp builtins.cpp fileio.cpp jseventmapper.cpp \
                         eventproxy.cpp slotproxy.cpp jseventutils.cpp qobject_binding.cpp \
                         kjsembed.cpp scalar_binding.cpp iosupport.cpp qwidget_binding.cpp \
                         widgetfactory.cpp qformbuilder_binding.cpp qpainter_binding.cpp \
                         settings.cpp svg_binding.cpp filedialog_binding.cpp \ 
                         brush.cpp color.cpp dom.cpp font.cpp image.cpp pen.cpp \
                         pixmap.cpp point.cpp rect.cpp size.cpp url.cpp 

kjsembedincludedir = $(includedir)/kjsembed
kjsembedinclude_HEADERS = global.h binding_support.h static_binding.h value_binding.h \
                          object_binding.h builtins.h fileio.h jseventmapper.h \
                          eventproxy.h slotproxy.h jseventutils.h qobject_binding.h \
                          kjsembed.h scalar_binding.h iosupport.h qwidget_binding.h \
                          widgetfactory.h qformbuilder_binding.h qpainter_binding.h \
                          settings.h svg_binding.h filedialog_binding.h \
                          brush.h color.h dom.h font.h image.h pen.h \
                          pixmap.h point.h rect.h size.h url.h pointer.h

libkjsembed_la_LDFLAGS = -version-info 1:0:0 -no-undefined $(all_libraries) $(KDE_RPATH)
libkjsembed_la_LIBADD = $(LIB_QT) $(LIB_QTNETWORK) $(LIB_QTDESIGNER) $(LIB_KDECORE) $(LIB_KDEUI) \
                        $(LIB_KFILE) $(LIB_KHTML) $(LIB_KJS) $(LIBSOCKET) $(LIB_KPARTS) $(LIB_QTXML) \
			-lQtSvg_debug
libkjsembed_la_METASOURCES = AUTO
