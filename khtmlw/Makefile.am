
lib_LTLIBRARIES = libkhtmlw.la
libkhtmlw_la_SOURCES = html.cpp htmlobj.cpp htmlfont.cpp gif.cpp jpeg.cpp formats.cpp htmlform.cpp ampseq.cpp
libkhtmlw_la_LIBADD = -lgif -ljpeg -L@kde_libraries@

BUILT_SOURCES = html.moc htmlform.moc
CLEANFILES = $(BUILT_SOURCES)
include_HEADERS = html.h htmlobj.h htmlfont.h formats.h htmlform.h ampseq.h
libkhtmlw_la_LDFLAGS = -version-info 0:5

MOC = @MOC@
INCLUDES = -I@qt_includes@ -I@kde_includes@ -I@x_includes@ -I.. -I
CXXFLAGS += -DSTDC_HEADERS -DHAVE_UNISTD_H 

########## Meta objects ##########

%.moc: %.h
	$(MOC) $< -o $@

##################################
# DO NOT DELETE
