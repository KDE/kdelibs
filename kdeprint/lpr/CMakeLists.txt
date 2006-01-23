kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kdeprint ${CMAKE_SOURCE_DIR}/kdeprint/management ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdeprint_lpr_PART_SRCS
printcapreader.cpp
lprhandler.cpp
matichandler.cpp
klprfactory.cpp
kmlprmanager.cpp
kmlpruimanager.cpp
lpchelper.cpp
lpqhelper.cpp
kmlprjobmanager.cpp
matichelper.cpp
klprprinterimpl.cpp
printcapentry.cpp
lprsettings.cpp
kmconfiglpr.cpp
apshandler.cpp
lprngtoolhandler.cpp
editentrydialog.cpp
)

kde4_automoc(${kdeprint_lpr_PART_SRCS})

kde4_add_plugin(kdeprint_lpr ${kdeprint_lpr_PART_SRCS})

target_link_libraries(kdeprint_lpr  ${QT_AND_KDECORE_LIBS} kdeprint_management kdefakes )

install_targets(/lib/kde4 kdeprint_lpr )


########### next target ###############

set(make_driver_db_lpr_SRCS
make_driver_db_lpr.c
${CMAKE_CURRENT_SOURCE_DIR}/../driverparse.c
)

kde4_automoc(${make_driver_db_lpr_SRCS})

kde4_add_executable(make_driver_db_lpr ${make_driver_db_lpr_SRCS})

target_link_libraries(make_driver_db_lpr  ${QT_AND_KDECORE_LIBS} kdefakes )

install_targets(/bin make_driver_db_lpr )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kdeprint FILES apsdriver1 apsdriver2 lprngtooldriver1 )
install_files( ${KDE4_DATA_DIR}/kdeprint/plugins FILES lpr.print )
install_files( /include/kdeprint/lpr FILES printcapentry.h lprhandler.h lprsettings.h )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -D_KDEPRINT_COMPILE
#
#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kdeprint -I$(top_srcdir)/kdeprint/management $(all_includes)
#
#kde_module_LTLIBRARIES = kdeprint_lpr.la
#
#kdeprint_lpr_la_SOURCES = printcapreader.cpp lprhandler.cpp \
#			     matichandler.cpp klprfactory.cpp kmlprmanager.cpp \
#			     kmlpruimanager.cpp lpchelper.cpp lpqhelper.cpp \
#			     kmlprjobmanager.cpp matichelper.cpp \
#			     klprprinterimpl.cpp printcapentry.cpp \
#			     lprsettings.cpp kmconfiglpr.cpp apshandler.cpp \
#			     lprngtoolhandler.cpp \
#			     editentrydialog.cpp
#kdeprint_lpr_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kdeprint_lpr_la_LIBADD = $(top_builddir)/kdeprint/management/libkdeprint_management.la $(top_builddir)/kdecore/libkdefakes.la
#kdeprint_lpr_la_METASOURCES = AUTO
#
#noinst_HEADERS = printcapreader.h \
#		 matichandler.h kmlprmanager.h \
#		 kmlpruimanager.h lpchelper.h lpqhelper.h \
#		 kmlprjobmanager.h matichelper.h klprprinterimpl.h \
#		 kmconfiglpr.h apshandler.h \
#		 lprngtoolhandler.h \
#		 editentrydialog.h
#
#kdeprintlpr_HEADERS = printcapentry.h lprhandler.h lprsettings.h
#kdeprintlprdir = $(includedir)/kdeprint/lpr
#
#bin_PROGRAMS = make_driver_db_lpr
#make_driver_db_lpr_SOURCES = make_driver_db_lpr.c
#make_driver_db_lpr_LDFLAGS = $(all_libraries)
#make_driver_db_lpr_LDADD = $(top_builddir)/kdecore/libkdefakes.la ../libdriverparse.a $(LIBDL)
#
#entry_DATA = lpr.print
#entrydir = $(kde_datadir)/kdeprint/plugins
#
#miscdata_DATA = apsdriver1 apsdriver2 lprngtooldriver1
#miscdatadir = $(kde_datadir)/kdeprint
