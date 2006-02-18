kde4_header()

add_subdirectory( cupsdconf2 ) 

include_directories( ${CUPS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/kdecore ${CMAKE_SOURCE_DIR}/kdecore/network ${CMAKE_SOURCE_DIR}/kio ${CMAKE_SOURCE_DIR}/kfile ${CMAKE_SOURCE_DIR}/kdeprint ${CMAKE_SOURCE_DIR}/kdeprint/management ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdeprint_cups_PART_SRCS
cupsinfos.cpp
ipprequest.cpp
kmcupsfactory.cpp
kmcupsjobmanager.cpp
kmcupsmanager.cpp
kmcupsuimanager.cpp
kmpropbanners.cpp
kmwbanners.cpp
kmwipp.cpp
kmwippselect.cpp
kcupsprinterimpl.cpp
kmcupsconfigwidget.cpp
kmconfigcups.cpp
kmconfigcupsdir.cpp
kmwippprinter.cpp
kmwfax.cpp
kmwquota.cpp
kmpropquota.cpp
kmwusers.cpp
kmpropusers.cpp
kpschedulepage.cpp
image.cpp
imagepreview.cpp
imageposition.cpp
kpimagepage.cpp
kptextpage.cpp
kphpgl2page.cpp
cupsaddsmb2.cpp
kmwother.cpp
ippreportdlg.cpp
kptagspage.cpp
)

if (UNIX)
   set(KDEFAKES_LIB kdefakes)
endif (UNIX)

kde4_automoc(${kdeprint_cups_PART_SRCS})

kde4_add_plugin(kdeprint_cups ${kdeprint_cups_PART_SRCS})

target_link_libraries(kdeprint_cups  ${QT_AND_KDECORE_LIBS} kdeprint_management kio ${KDEFAKES_LIB} ${CUPS_LIBRARIES} )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdeprint_cups )
install_targets(${PLUGIN_INSTALL_DIR} kdeprint_cups )


########### next target ###############

set(make_driver_db_cups_SRCS
make_driver_db_cups.c
${CMAKE_CURRENT_SOURCE_DIR}/../driverparse.c
)

kde4_automoc(${make_driver_db_cups_SRCS})

kde4_add_executable(make_driver_db_cups ${make_driver_db_cups_SRCS})

target_link_libraries(make_driver_db_cups  ${QT_AND_KDECORE_LIBS} ${KDEFAKES_LIB} )

install_targets(/bin make_driver_db_cups )


########### next target ###############

set(cupsdoprint_SRCS
cupsdoprint.c
)

kde4_automoc(${cupsdoprint_SRCS})

kde4_add_executable(cupsdoprint ${cupsdoprint_SRCS})

target_link_libraries(cupsdoprint  ${QT_AND_KDECORE_LIBS} ${KDEFAKES_LIB} ${CUPS_LIBRARIES})

install_targets(/bin cupsdoprint )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdeprint FILES preview.png preview-mini.png )
install_files( ${DATA_INSTALL_DIR}/kdeprint/plugins FILES cups.print )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -D_KDEPRINT_COMPILE
#
#SUBDIRS = cupsdconf2
#
#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdecore/network -I$(top_srcdir)/kio -I$(top_srcdir)/kfile -I$(top_srcdir)/kdeprint -I$(top_srcdir)/kdeprint/management $(all_includes)
#
#kde_module_LTLIBRARIES = kdeprint_cups.la
#
#kdeprint_cups_la_SOURCES = cupsinfos.cpp ipprequest.cpp kmcupsfactory.cpp kmcupsjobmanager.cpp \
#			      kmcupsmanager.cpp kmcupsuimanager.cpp kmpropbanners.cpp kmwbanners.cpp kmwipp.cpp \
#			      kmwippselect.cpp kcupsprinterimpl.cpp kmcupsconfigwidget.cpp \
#			      kmconfigcups.cpp kmconfigcupsdir.cpp kmwippprinter.cpp kmwfax.cpp kmwquota.cpp \
#			      kmpropquota.cpp kmwusers.cpp kmpropusers.cpp kpschedulepage.cpp image.cpp \
#				  imagepreview.cpp imageposition.cpp kpimagepage.cpp \
#				  kptextpage.cpp kphpgl2page.cpp cupsaddsmb2.cpp kmwother.cpp ippreportdlg.cpp \
#				  kptagspage.cpp
#kdeprint_cups_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kdeprint_cups_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la $(LIB_KIO) $(LIB_CUPS) $(top_builddir)/kdecore/libkdefakes.la
#kdeprint_cups_la_METASOURCES = AUTO
#
#noinst_HEADERS = cupsinfos.h ipprequest.h kmcupsjobmanager.h kmcupsmanager.h kmcupsuimanager.h \
#		 kmpropbanners.h kmwbanners.h kmwipp.h kmwippselect.h kcupsprinterimpl.h \
#		 kmcupsconfigwidget.h kmconfigcups.h kmconfigcupsdir.h kmwippprinter.h kmwfax.h kmwquota.h \
#		 kmpropquota.h kmwusers.h kmpropusers.h kpschedulepage.h imagepreview.h imageposition.h \
#		 kpimagepage.h kptextpage.h kphpgl2page.h cupsaddsmb2.h \
#		 kmwother.h ippreportdlg.h kptagspage.h
#
#bin_PROGRAMS = make_driver_db_cups cupsdoprint
#make_driver_db_cups_SOURCES = make_driver_db_cups.c
#make_driver_db_cups_LDADD = ../libdriverparse.a $(top_builddir)/kdecore/libkdefakes.la $(LIBDL) $(LIBZ)
#make_driver_db_cups_LDFLAGS = $(all_libraries)
#
#cupsdoprint_SOURCES = cupsdoprint.c
#cupsdoprint_LDADD = $(LIB_CUPS) $(top_builddir)/kdecore/libkdefakes.la
#cupsdoprint_LDFLAGS = $(all_libraries)
#
#entry_DATA = cups.print
#entrydir = $(kde_datadir)/kdeprint/plugins
#
#misc_DATA = preview.png preview-mini.png
#miscdir = $(kde_datadir)/kdeprint
#
#EXTRA_DIST = postscript.ppd.gz
#
#install-data-local:
#	if test -n "$(cups_modeldir)"; then \
#		$(INSTALL_DATA) $(srcdir)/postscript.ppd.gz $(DESTDIR)$(cups_modeldir)/postscript.ppd.gz || true ;\
#	fi
#
