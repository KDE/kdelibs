
lib_LTLIBRARIES = libkhtmlw.la
libkhtmlw_la_SOURCES = html.cpp htmlobj.cpp htmlfont.cpp gif.cpp jpeg.cpp formats.cpp htmlform.cpp ampseq.cpp
libkhtmlw_la_LIBADD = -lkdecore -lqt -lX11 -lgif -ljpeg -L@topdir@/kdecore/.libs -L@topdir@/kdeui/.libs

BUILT_SOURCES = html.moc htmlform.moc
CLEANFILES = $(BUILT_SOURCES)
include_HEADERS = html.h htmlobj.h htmlfont.h formats.h htmlform.h ampseq.h
libkhtmlw_la_LDFLAGS = -version-info 0:5 -rpath $(libdir)

MOC = @MOC@
INCLUDES = -I../kdecore -I../kdeui @X_INCLUDES@ @QT_INCLUDES@
########## Meta objects ##########

%.moc: %.h
	$(MOC) $< -o $@

##################################
# DO NOT DELETE
