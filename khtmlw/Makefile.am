
lib_LTLIBRARIES = libkhtmlw.la
libkhtmlw_la_SOURCES = jscript.cpp html.cpp htmlobj.cpp htmlfont.cpp gif.cpp jpeg.cpp formats.cpp htmlform.cpp ampseq.cpp htmltoken.cpp htmlframe.cpp htmlview.cpp htmlclue.cpp debug.cpp
libkhtmlw_la_LIBADD = -L@topdir@/kdecore/_libs -L@topdir@/kdeui/_libs -L@topdir@/jscript/_libs @all_libraries@ -lkdecore -lqt -lX11 -lgif @LIBJPEG@

BUILT_SOURCES = html.moc htmlobj.moc htmlform.moc htmlframe.moc htmlview.moc
html.cpp: html.moc
htmlobj.cpp: htmlobj.moc
htmlform.cpp: htmlform.moc
htmlframe.cpp: htmlframe.moc
htmlview.cpp: htmlview.moc

CLEANFILES = $(BUILT_SOURCES)
include_HEADERS = jscript.h html.h htmlobj.h htmlfont.h formats.h htmlform.h ampseq.h htmltoken.h htmlframe.h htmlview.h htmlclue.h
libkhtmlw_la_LDFLAGS = -version-info 0:10 #  -rpath $(libdir)

data_DATA = khtmlw_dnd.xpm
datadir = $(prefix)/share/apps/khtmlw/pics

MOC = @MOC@
INCLUDES = -I../kdecore -I../kdeui -I../jscript @all_includes@ 

html.moc: html.h
	$(MOC) -o html.moc html.h 

htmlobj.moc: htmlobj.h
	$(MOC) -o htmlobj.moc htmlobj.h 

htmlform.moc: htmlform.h
	$(MOC) -o htmlform.moc htmlform.h 

htmlframe.moc: htmlframe.h
	$(MOC) -o htmlframe.moc htmlframe.h 

htmlview.moc: htmlview.h
	$(MOC) -o htmlview.moc htmlview.h 

