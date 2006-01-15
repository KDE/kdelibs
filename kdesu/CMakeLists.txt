kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kio/ ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdesu_LIB_SRCS
client.cpp
process.cpp
kcookie.cpp
su.cpp
ssh.cpp
stub.cpp
kdesu_pty.cpp
)

kde4_automoc(${kdesu_LIB_SRCS})

kde4_add_library(kdesu SHARED ${kdesu_LIB_SRCS})

target_link_libraries(kdesu  ${QT_AND_KDECORE_LIBS} )

set_target_properties(kdesu PROPERTIES VERSION 4.2.0 SOVERSION 4 )
install_targets(/lib kdesu )


########### next target ###############

set(kdesu_stub_SRCS
kdesu_stub.c
)

kde4_automoc(${kdesu_stub_SRCS})

kde4_add_executable(kdesu_stub ${kdesu_stub_SRCS})

target_link_libraries(kdesu_stub  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kdesu_stub )


########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for libkdesu
#
#INCLUDES = -I$(top_srcdir)/kio/ $(all_includes)
#
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#lib_LTLIBRARIES = libkdesu.la
#libkdesu_la_SOURCES = client.cpp process.cpp kcookie.cpp su.cpp ssh.cpp stub.cpp kdesu_pty.cpp
#libkdesu_la_LDFLAGS = -version-info 6:0:2 -no-undefined $(all_libraries) 
#libkdesu_la_LIBADD = $(LIB_KDECORE) $(LIBUTIL)
#libkdesu_la_NMCHECK = $(srcdir)/libkdesu.nmcheck
#libkdesu_la_NMCHECKWEAK = $(srcdir)/libkdesu_weak.nmcheck $(top_srcdir)/kdecore/libkdecore_weak.nmcheck \
#	$(top_srcdir)/dcop/libDCOP_weak.nmcheck $(top_srcdir)/kdecore/libqt-mt_weak.nmcheck
#
#kdesudir = $(includedir)/kdesu
#kdesu_HEADERS = defaults.h client.h process.h kdesu_pty.h kcookie.h su.h ssh.h stub.h
#
#install-data-hook:
#	$(mkinstalldirs) $(DESTDIR)$(kdesudir)
#	-rm -f $(DESTDIR)$(kdesudir)/pty.h
#	ln -s kdesu_pty.h $(DESTDIR)$(kdesudir)/pty.h
#
#uninstall-local:
#	-rm -f $(DESTDIR)$(kdesudir)/pty.h
#
#bin_PROGRAMS = kdesu_stub
#kdesu_stub_SOURCES = kdesu_stub.c
#
#include $(top_srcdir)/admin/Doxyfile.am
#
