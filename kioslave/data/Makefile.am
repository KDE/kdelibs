####### kdevelop will overwrite this part!!! (begin)##########
kde_module_LTLIBRARIES = kio_data.la


INCLUDES = $(all_includes)

kio_data_la_METASOURCES=AUTO

kio_data_la_SOURCES = data.cpp 
kio_data_la_LIBADD = $(LIB_KIO)
# $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT)

kio_data_la_LDFLAGS = -module $(KDE_PLUGIN)
#$(all_libraries) 

noinst_HEADERS = data.h

fileincludedir = $(includedir)
fileinclude_HEADERS = data.h

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = data.protocol

#EXTRA_DIST = data.cpp data.h data.protocol 
#
#install-data-local:
#	$(mkinstalldirs)  $(kde_servicesdir)/
#	$(INSTALL_DATA) $(srcdir)/data.protocol  $(kde_servicesdir)/data.protocol
#
#uninstall-local:
#	-rm -f  $(kde_servicesdir)/data.protocol

####### kdevelop will overwrite this part!!! (end)############
# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).


# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
		 $(XGETTEXT) -C -ki18n -x $(kde_includes)/kde.pot $$LIST -o ../po/data.pot; \
	fi

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
