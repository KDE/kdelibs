lib_LTLIBRARIES = libqtmcop.la

INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop
libqtmcop_la_SOURCES = qiomanager.cc

METASOURCES = qiomanager.moc

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = qiomanager.h

#
# -frepo closure (experimental)
#
closure: $(libqtmcop_la_OBJECTS)
	@echo "Building -frepo compliant library closure"
	@echo "int main() {}" > closure.cc
	$(CXXLINK) -o closure $(libqtmcop_la_OBJECTS) $(LIB_QT) closure.cc
	@echo "closure timestamp" > closure
	@rm -f closure.cc
