kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(artskde_LIB_SRCS
artskde.cc
kioinputstream_impl.cpp
kplayobject.cc
kplayobjectfactory.cc
kartsfloatwatch.cpp
kartsdispatcher.cc
kaudiorecordstream.cpp
kaudioplaystream.cpp
kartsserver.cpp
kdatarequest_impl.cpp
kaudioconverter.cc
kvideowidget.cpp
kplayobjectcreator.cc
kaudiomanagerplay.cpp
)

kde4_automoc(${artskde_LIB_SRCS})

kde4_add_library(artskde SHARED ${artskde_LIB_SRCS})

target_link_libraries(artskde  ${QT_AND_KDECORE_LIBS} kio qtmcop soundserver_idl )

set_target_properties(artskde PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install_targets(${KDE4_LIB_INSTALL_DIR} artskde )


########### next target ###############

set(kiotest_SRCS
kiotest.cc
)

kde4_automoc(${kiotest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kiotest ${kiotest_SRCS})

target_link_libraries(kiotest  ${QT_AND_KDECORE_LIBS} qtmcop kmedia2 soundserver_idl )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kiotestslow_SRCS
kiotestslow.cc
)

kde4_automoc(${kiotestslow_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kiotestslow ${kiotestslow_SRCS})

target_link_libraries(kiotestslow  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kconverttest_SRCS
kconverttest.cc
)

kde4_automoc(${kconverttest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kconverttest ${kconverttest_SRCS})

target_link_libraries(kconverttest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############
install_files( /include/arts FILES kplayobject.h kplayobjectfactory.h kartsfloatwatch.h                           artskde.h kartsdispatcher.h                           kaudiorecordstream.h kaudioplaystream.h                           kartsserver.h kvideowidget.h kaudiomanagerplay.h )


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_builddir)/arts/kde \
#           -I$(top_srcdir) -I$(includedir)/arts $(all_includes)
#
#lib_LTLIBRARIES = libartskde.la
#libartskde_la_SOURCES = artskde.cc kioinputstream_impl.cpp kplayobject.cc \
#                        kplayobjectfactory.cc kartsfloatwatch.cpp kartsdispatcher.cc \
#                        kaudiorecordstream.cpp kaudioplaystream.cpp \
#                        kartsserver.cpp kdatarequest_impl.cpp kaudioconverter.cc \
#                        kvideowidget.cpp kplayobjectcreator.cc \
#                        kaudiomanagerplay.cpp
#libartskde_la_LIBADD = $(LIB_KIO) -lqtmcop -lsoundserver_idl
#libartskde_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
#libartskde_la_METASOURCES = AUTO
#libartskde_la_COMPILE_FIRST = artskde.h
#
#artskdeincludedir = $(includedir)/arts
#artskdeinclude_HEADERS = kplayobject.h kplayobjectfactory.h kartsfloatwatch.h \
#                         artskde.h kartsdispatcher.h \
#                         kaudiorecordstream.h kaudioplaystream.h \
#                         kartsserver.h kvideowidget.h kaudiomanagerplay.h
#noinst_HEADERS = kplayobjectcreator.h kplayobjectfactory_p.h
#
#artskde.mcoptype: artskde.h
#artskde.mcopclass: artskde.h
#MCOPINC = -I$(srcdir)
#artskde.cc artskde.h: $(top_srcdir)/arts/kde/artskde.idl $(MCOPIDL)
#	     $(MCOPIDL) -I$(includedir)/arts -t $(MCOPINC) $(top_srcdir)/arts/kde/artskde.idl
#
#DISTCLEANFILES = artskde.cc artskde.h artskde.mcoptype artskde.mcopclass
#
#check_PROGRAMS = kiotest kiotestslow kconverttest
#kiotest_SOURCES = kiotest.cc
#kiotestslow_SOURCES = kiotestslow.cc
#kconverttest_SOURCES = kconverttest.cc
#
#kiotest_LDADD = libartskde.la -lqtmcop -lkmedia2 -lsoundserver_idl
#kiotestslow_LDADD = $(kiotest_LDADD)
#kconverttest_LDADD = $(kiotest_LDADD)
#
## vim: ts=8 noet
