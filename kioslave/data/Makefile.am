####### kdevelop will overwrite this part!!! (begin)##########
kde_module_LTLIBRARIES = kio_data.la


INCLUDES = $(all_includes)

kio_data_la_METASOURCES=AUTO

kio_data_la_SOURCES = data.cpp 
kio_data_la_LIBADD = $(LIB_KIO)

kio_data_la_LDFLAGS = -module $(KDE_PLUGIN)

noinst_HEADERS = data.h

fileincludedir = $(includedir)
fileinclude_HEADERS = data.h

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = data.protocol

test: testkio_data

testkio_data.moc: testkio_data.cpp data.cpp data.h
	$(MOC) -i -o $@ $< && touch $@

testkio_data: testkio_data.cpp testkio_data.moc
		$(CXX) -DTESTKIO -O0 -g3 $(KDE_INCLUDES) $(QT_INCLUDES) \
		$(KDE_LDFLAGS) $(QT_LDFLAGS) $(X_LDFLAGS) -I. \
		$< -o $@ -lkio $(LIB_QT)
	
clean-local:
	rm -f testkio_data testkio_data.moc

.PHONY: test
