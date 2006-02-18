kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kio ${CMAKE_SOURCE_DIR}/kfile ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(cupsdconf_KDEINIT_SRCS
cupsdconf.cpp
cupsddialog.cpp
cupsdpage.cpp
cupsdcomment.cpp
cupsdsplash.cpp
cupsdserverpage.cpp
cupsdlogpage.cpp
cupsdjobspage.cpp
cupsdfilterpage.cpp
qdirlineedit.cpp
cupsddirpage.cpp
portdialog.cpp
cupsdnetworkpage.cpp
editlist.cpp
cupsdbrowsingpage.cpp
browsedialog.cpp
cupsdsecuritypage.cpp
locationdialog.cpp
addressdialog.cpp
cups-util.c
qdirmultilineedit.cpp
sizewidget.cpp
main.cpp
)

kde4_automoc(${cupsdconf_KDEINIT_SRCS})

kde4_add_kdeinit_executable( cupsdconf ${cupsdconf_KDEINIT_SRCS})

target_link_libraries(kdeinit_cupsdconf  ${QT_AND_KDECORE_LIBS} kio ${CUPS_LIBRARIES})

install_targets(${LIB_INSTALL_DIR} kdeinit_cupsdconf )

target_link_libraries( cupsdconf kdeinit_cupsdconf )
install_targets(/bin cupsdconf )

########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdeprint FILES kde_logo.png cups_logo.png cupsd.conf.template )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kio -I$(top_srcdir)/kfile $(all_includes)
#
#bin_PROGRAMS =
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = cupsdconf.la
#
## library creation
#cupsdconf_la_SOURCES = cupsdconf.cpp cupsddialog.cpp cupsdpage.cpp cupsdcomment.cpp cupsdsplash.cpp \
#			  cupsdserverpage.cpp cupsdlogpage.cpp cupsdjobspage.cpp cupsdfilterpage.cpp \
#			  qdirlineedit.cpp cupsddirpage.cpp portdialog.cpp cupsdnetworkpage.cpp \
#			  editlist.cpp cupsdbrowsingpage.cpp browsedialog.cpp cupsdsecuritypage.cpp \
#			  locationdialog.cpp addressdialog.cpp cups-util.c \
#			  qdirmultilineedit.cpp sizewidget.cpp main.cpp
#cupsdconf_la_METASOURCES = AUTO
#cupsdconf_la_LDFLAGS = -avoid-version -module $(all_libraries) $(KDE_RPATH)
#cupsdconf_la_LIBADD = $(LIB_KIO) $(LIB_CUPS)
#
#noinst_HEADERS = cupsdconf.h cupsdpage.h cupsddialog.h cupsdcomment.h cupsdsplash.h \
#		 cupsdserverpage.h cupsdlogpage.h cupsdjobspage.h cupsdfilterpage.h \
#		 qdirlineedit.h cupsddirpage.h portdialog.h cupsdnetworkpage.h \
#		 editlist.h cupsdbrowsingpage.h browsedialog.h cupsdsecuritypage.h \
#		 locationdialog.h addressdialog.h cups-util.h qdirmultilineedit.h \
#		 sizewidget.h
#
## Data
#cupsdconfdata_DATA = kde_logo.png cups_logo.png cupsd.conf.template
#cupsdconfdatadir   = $(kde_datadir)/kdeprint
#
## Translations
#messages:
#	perl cupsdcomment.pl < cupsd.conf.template > cupsd.conf.template.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/cupsdconf.pot
#	rm -f cupsd.conf.template.cpp
