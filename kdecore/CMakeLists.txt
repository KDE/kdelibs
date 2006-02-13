kde4_header()

project(kdecore)

include_directories( ${CMAKE_SOURCE_DIR}/kdefx ${CMAKE_SOURCE_DIR}/kdecore/network ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


# kdecore_OPTIONAL_SRCS is used to collect source files
# which are not always compiled into kdecore
# if it's used, *always* append to it
set(kdecore_OPTIONAL_SRCS)


# you can find macro_optional_find_package() in kdelibs/cmake/modules/OptionalFindPackage.cmake
# it is the same as FIND_PACKAGE(<name>) but additionally creates an OPTION(WITH_<name>)
# so the checking for the software can be disabled via ccmake or -DWITH_<name>=OFF
macro_optional_find_package(AGG)
if (AGG_FOUND)
   include_directories(${AGG_INCLUDE_DIR})
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS} svgicons/ksvgiconengine.cpp svgicons/ksvgiconpainter.cpp )
	install_files( /include FILES svgicons/ksvgiconengine.h )
endif (AGG_FOUND)


# compile bzip2 support if available
if(BZIP2_FOUND)
  include_directories(${BZIP2_INCLUDE_DIR})
  set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS} kbzip2filter.cpp )
endif(BZIP2_FOUND)


# compile zlib support if available
if(ZLIB_FOUND)
   include_directories(${ZLIB_INCLUDE_DIR})
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS} kgzipfilter.cpp )
endif(ZLIB_FOUND)


if (WIN32)
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
      ${CMAKE_SOURCE_DIR}/libltdl/ltdl_win.c
      netsupp_win32.cpp
      kapplication_win.cpp 
      kglobalaccel_win.cpp
      ktoolinvocation_win.cpp )
endif (WIN32)


if (UNIX)
   set(kdecore_OPTIONAL_SRCS ${kdecore_OPTIONAL_SRCS}
      kglobalaccel_x11.cpp 
      ktoolinvocation_x11.cpp 
      ksocks.cpp
      netsupp.cpp
      netwm.cpp
      kpty.cpp
      kwin.cpp
      kwinmodule.cpp 
      network/khttpproxysocketdevice.cpp
      network/ksockssocketdevice.cpp
      network/knetworkinterface.cpp )
endif (UNIX)


add_subdirectory( malloc )
add_subdirectory( kconfig_compiler )
add_subdirectory( tests )
add_subdirectory( network )


########### next target ###############

if (NOT WIN32)

   kde4_add_library(kdefakes SHARED fakes.c)
   set_target_properties(kdefakes PROPERTIES VERSION 4.2.0 SOVERSION 4 )
   install_targets(${KDE4_LIB_INSTALL_DIR} kdefakes )

   #no ttyname on windows
   kde4_add_executable(kgrantpty kgrantpty.c)
   target_link_libraries(kgrantpty  kdefakes )
   install_targets(/bin kgrantpty )
   
endif (NOT WIN32)



########### next target ###############

set(kdecore_LIB_SRCS
   kapplication.cpp
   kauthorized.cpp
   kdebug.cpp
   kconfigbase.cpp
   kconfig.cpp
   ksimpleconfig.cpp
   kconfigbackend.cpp
   kmanagerselection.cpp
   kdesktopfile.cpp
   kstandarddirs.cpp
   kprocess.cpp
   kprocctrl.cpp
   klocale.cpp
   krfcdate.cpp
   kiconeffect.cpp
   kicontheme.cpp
   kiconloader.cpp
   krootprop.cpp
   kcharsets.cpp
   kckey.cpp
   kshortcut.cpp
   kkeynative_x11.cpp
   kkeyserver_x11.cpp
   kaccelaction.cpp
   kshortcutmenu.cpp
   kaccelbase.cpp
   kaccel.cpp
   kglobalaccel.cpp
   kstdaccel.cpp
   kshortcutlist.cpp
   kcrash.cpp
   kurl.cpp
   kglobal.cpp
   kglobalsettings.cpp
   kallocator.cpp
   kvmallocator.cpp
   kmimesourcefactory.cpp
   kinstance.cpp
   kpalette.cpp
   kipc.cpp
   klibloader.cpp
   ktempfile.cpp
   kuniqueapplication.cpp
   kacceleratormanager.cpp
   ksavefile.cpp
   krandomsequence.cpp
   kstringhandler.cpp
   kcompletion.cpp
   kcmdlineargs.cpp
   kaboutdata.cpp
   kcompletionbase.cpp
   knotifyclient.cpp
   kaudioplayer.cpp
   kdcoppropertyproxy.cpp
   kprocio.cpp
   krandom.cpp
   kpixmapprovider.cpp
   ktoolinvocation.cpp
   kcodecs.cpp
   ksycoca.cpp
   ksycocadict.cpp
   ksycocafactory.cpp
   kxmessages.cpp
   kstartupinfo.cpp
   kcatalog.cpp
   kstaticdeleter.cpp
   kappdcopiface.cpp
   kclipboard.cpp
   kcheckaccelerators.cpp
   kdeversion.cpp
   kdebugdcopiface.cpp
   kcalendarsystem.cpp
   kcalendarsystemgregorian.cpp
   kcalendarsystemhijri.cpp
   kcalendarsystemhebrew.cpp
   kcalendarsystemfactory.cpp
   kmacroexpander.cpp
   kidna.cpp
   ktempdir.cpp
   kshell.cpp
   kmountpoint.cpp
   kcalendarsystemjalali.cpp
   kprotocolinfo_kdecore.cpp
   kprotocolinfofactory.cpp
   kxerrorhandler.cpp
   kuser.cpp
   kconfigskeleton.cpp
   klockfile.cpp
   ktimezones.cpp
   ktzfiletimezone.cpp
   kdatetime.cpp
   kmath.cpp
   kfilterbase.cpp
   kfilterdev.cpp
   kdedmodule.cpp
   network/kresolver.cpp
   network/kresolvermanager.cpp
   network/kresolverworkerbase.cpp
   network/ksocketaddress.cpp
   network/kresolverstandardworkers.cpp
   network/kreverseresolver.cpp
   network/ksocketdevice.cpp
   network/ksocketbase.cpp
   network/kclientsocketbase.cpp
   network/kstreamsocket.cpp
   network/kserversocket.cpp
   network/kdatagramsocket.cpp
   network/kbufferedsocket.cpp
   network/ksocketbuffer.cpp
   ${kdecore_OPTIONAL_SRCS}
   malloc/malloc.c
   ${CMAKE_SOURCE_DIR}/libltdl/ltdl.c
)

kde4_automoc(${kdecore_LIB_SRCS})

set( kdecore_DCOP_SKEL_SRCS
   ksycoca.h
   kappdcopiface.h
   kdebugdcopiface.h
   kdedmodule.h
)

if(UNIX)
    set(kdecore_LIB_SRCS ${kdecore_LIB_SRCS} fakes.c)
endif(UNIX)

kde4_add_dcop_skels(kdecore_LIB_SRCS ${kdecore_DCOP_SKEL_SRCS})

kde4_add_library(kdecore SHARED ${kdecore_LIB_SRCS})

target_link_libraries(kdecore  ${QT_QTCORE_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTGUI_LIBRARY} ${ZLIB_LIBRARIES} ${X11_LIBRARIES} ${UTIL_LIBRARY} DCOP ${CARBON_LIBRARY})

if (WIN32)
    add_definitions(-DKIO_EXPORT=)
endif (WIN32)

if(HAVE_IDNA_H)
   target_link_libraries(kdecore idn)
endif(HAVE_IDNA_H)

if(BZIP2_FOUND)
   target_link_libraries(kdecore  ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)

if(AGG_FOUND)
   target_link_libraries(kdecore  ${QT_QTXML_LIBRARY} ${AGG_LIBRARIES})
endif(AGG_FOUND)

if(HAVE_LIBDL)
   target_link_libraries(kdecore dl)
endif(HAVE_LIBDL)

set_target_properties(kdecore PROPERTIES VERSION 4.2.0 SOVERSION 4 )
install_targets(${KDE4_LIB_INSTALL_DIR} kdecore )


########### next target ###############

set(prefix ${KDE4_DIR})
set(datadir ${KDE4_DIR}${KDE4_DATA_DIR})
set(exec_prefix ${KDE4_DIR}/bin)
set(libdir ${KDE4_DIR}/lib)
set(includedir ${KDE4_DIR}/include)
set(kde_appsdir ${KDE4_DIR}${KDE4_APPS_DIR} )
set(kde_bindir ${KDE4_DIR}/bin )
set(kde_confdir ${KDE4_DIR}${KDE4_CONFIG_DIR})
set(kde_datadir ${KDE4_DIR}${KDE4_CONFIG_DIR})
set(kde_htmldir ${KDE4_DIR}${KDE4_HTML_DIR})
set(kde_icondir ${KDE4_DIR}${KDE4_ICON_DIR})
set(kde_kcfgdir ${KDE4_DIR}${KDE4_KCFG_DIR})
set(kde_locale  ${KDE4_DIR}${KDE4_LOCALE_DIR})
set(kde_moduledir ${KDE4_DIR}/lib/kde3)
set(kde_mimedir ${KDE4_DIR}${KDE4_MIME_DIR})
set(kde_servicesdir ${KDE4_DIR}${KDE4_SERVICES_DIR})
set(kde_servicetypesdir ${KDE4_DIR}${KDE4_SERVICETYPES_DIR})
set(kde_sounddir ${KDE4_DIR}${KDE4_SOUND_DIR})
set(kde_templatesdir ${KDE4_DIR}${KDE4_TEMPLATES_DIR})
set(kde_wallpaperdir ${KDE4_DIR}${KDE4_WALLPAPER_DIR})
set(xdg_appsdir  ${KDE4_DIR}${XDG_APPS_DIR})
set(xdg_directorydir ${KDE4_DIR}${XDG_DIRECTORY_DIR})


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kde-config.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/kde-config.cpp @ONLY)
macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kde-config.cpp  )

kde4_add_executable(kde-config ${CMAKE_CURRENT_BINARY_DIR}/kde-config.cpp )

target_link_libraries(kde-config ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kde-config)


########### install files ###############
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/all_languages.desktop ${CMAKE_CURRENT_BINARY_DIR}/all_languages @ONLY)
install_files( ${KDE4_LOCALE_DIR} FILES all_languages )
install_files( ${KDE4_DATA_DIR}/knotify FILES eventsrc )
install_files( ${KDE4_CONFIG_DIR} FILES kdebug.areas kdebugrc language.codes )
install_files( ${KDE4_CONFIG_DIR}/colors FILES 40.colors Web.colors Royal.colors Rainbow.colors )
install_files( /include FILES kconfig.h kconfigskeleton.h   kconfigdata.h ksimpleconfig.h    kconfigbase.h kdesktopfile.h kurl.h kaboutdata.h   kcmdlineargs.h kconfigbackend.h kapplication.h kauthorized.h ktoolinvocation.h   kuniqueapplication.h kcharsets.h kdeversion.h kpty.h kprocess.h   kprocctrl.h klocale.h kicontheme.h kiconloader.h kdebug.h   kwinmodule.h kwin.h krootprop.h kshortcut.h kkeynative.h kaccel.h    kglobalaccel.h kstdaccel.h kshortcutlist.h kcatalog.h    kcompletion.h kstringhandler.h   kstandarddirs.h kglobal.h kglobalsettings.h ksharedptr.h    kallocator.h kvmallocator.h kcrash.h krfcdate.h    kinstance.h kpalette.h kipc.h klibloader.h ktempfile.h ksavefile.h   krandomsequence.h knotifyclient.h kiconeffect.h    kaudioplayer.h kdcoppropertyproxy.h netwm.h kacceleratormanager.h    netwm_def.h kpixmapprovider.h kstaticdeleter.h     kprocio.h kasyncio.h krandom.h   kbufferedio.h kmimesourcefactory.h kcodecs.h ksocks.h    ksycoca.h ksycocaentry.h ksycocatype.h kxmessages.h kstartupinfo.h   kgenericfactory.h kgenericfactory.tcc     ktypelist.h ksortablevaluelist.h kclipboard.h   kcalendarsystem.h kcalendarsystemfactory.h kmacroexpander.h    kmanagerselection.h kmountpoint.h kuser.h klockfile.h    kidna.h ktempdir.h kshell.h fixx11h.h kxerrorhandler.h   kde_file.h ktimezones.h ktzfiletimezone.h kmath.h     kfilterbase.h kfilterdev.h kdatetime.h qtest_kde.h conversion_check.h   kdedmodule.h )

kde4_create_doxygen_docs( REFERENCES dcop kdeui kparts kio EXCLUDE malloc kde_file.h  )

kde4_footer()



#original Makefile.am contents follow:

#
##    This file is part of the KDE libraries
##
##    Copyright (C) 1996-1997 Matthias Kalle Dalheimer (kalle@kde.org)
##			  (C) 1997 Stephan Kulow (coolo@kde.org)
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Library General Public
##    License as published by the Free Software Foundation; either
##    version 2 of the License, or (at your option) any later version.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Library General Public License for more details.
#
##    You should have received a copy of the GNU Library General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#INCLUDES = -I$(top_srcdir)/kdefx -I$(top_srcdir)/kdecore/network $(all_includes) $(AGG_CFLAGS)
#
#if with_agg
#SVGICONS=svgicons
#SVGICON_LIB=svgicons/libkdesvgicons.la
#endif
#
#SUBDIRS = malloc $(SVGICONS) . kconfig_compiler tests network
#
## For the future: examine if condensing the tons of *_LDFLAGS variables
## into $(all_libraries) isn't better
#AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)
#
#lib_LTLIBRARIES = libkdefakes.la libkdecore.la
#lib_LIBRARIES = libkdefakes_nonpic.a
#
#include_HEADERS = kconfig.h kconfigskeleton.h \
#	kconfigdata.h ksimpleconfig.h \
#	kconfigbase.h kdesktopfile.h kurl.h kaboutdata.h \
#	kcmdlineargs.h kconfigbackend.h kapplication.h kauthorized.h ktoolinvocation.h \
#	kuniqueapplication.h kcharsets.h kdeversion.h kpty.h kprocess.h \
#	kprocctrl.h klocale.h kicontheme.h kiconloader.h kdebug.h \
#	kwinmodule.h kwin.h krootprop.h kshortcut.h kkeynative.h kaccel.h \
#	kglobalaccel.h kstdaccel.h kshortcutlist.h kcatalog.h \
#	kcompletion.h kstringhandler.h \
#	kstandarddirs.h kglobal.h kglobalsettings.h ksharedptr.h \
#	kallocator.h kvmallocator.h kcrash.h krfcdate.h \
#	kinstance.h kpalette.h kipc.h klibloader.h ktempfile.h ksavefile.h \
#	krandomsequence.h knotifyclient.h kiconeffect.h \
#	kaudioplayer.h kdcoppropertyproxy.h netwm.h kacceleratormanager.h \
#	netwm_def.h kpixmapprovider.h kstaticdeleter.h \
#	kprocio.h kasyncio.h krandom.h\
#	kbufferedio.h kmimesourcefactory.h kcodecs.h ksocks.h \
#	ksycoca.h ksycocaentry.h ksycocatype.h kxmessages.h kstartupinfo.h \
#	kgenericfactory.h kgenericfactory.tcc \
#	ktypelist.h ksortablevaluelist.h kclipboard.h \
#	kcalendarsystem.h kcalendarsystemfactory.h kmacroexpander.h \
#	kmanagerselection.h kmountpoint.h kuser.h klockfile.h \
#	kidna.h ktempdir.h kshell.h fixx11h.h kxerrorhandler.h \
#	kde_file.h ktimezones.h ktzfiletimezone.h kmdcodec.h kmath.h \
#	kfilterbase.h kfilterdev.h kdatetime.h qtest_kde.h conversion_check.h \
#	kdedmodule.h
#
#libkdefakes_la_SOURCES = fakes.c
#libkdefakes_la_LDFLAGS = -version-info 6:0:2
#libkdefakes_la_LIBADD  = $(LIBSOCKET)
#
#libkdefakes_nonpic_a_SOURCES = fakes_nonpic.c
#
#fakes_nonpic.c:$(srcdir)/fakes.c
#	-rm -f fakes_nonpic.c
#	$(LN_S) $(srcdir)/fakes.c fakes_nonpic.c
#
#noinst_HEADERS = kaccelaction.h kaccelbase.h kaccelprivate.h kckey.h \
#	kcompletion_private.h netwm_p.h \
#	kglobalaccel_x11.h kglobalaccel_win.h kkeyserver_x11.h kkeyserver.h \
#	kshortcutmenu.h ksycocadict.h ksycocafactory.h netsupp.h \
#	kcheckaccelerators.h kcalendarsystemgregorian.h \
#	kcalendarsystemhijri.h kcalendarsystemhebrew.h kcalendarsystemjalali.h \
#	kprotocolinfofactory.h
#
#libkdecore_la_SOURCES = kapplication.cpp kauthorized.cpp ktoolinvocation_x11.cpp \
#	kdebug.cpp netwm.cpp kconfigbase.cpp kconfig.cpp  ksimpleconfig.cpp \
#	kconfigbackend.cpp kmanagerselection.cpp kdesktopfile.cpp \
#	kstandarddirs.cpp kpty.cpp kprocess.cpp kprocctrl.cpp \
#	klocale.cpp krfcdate.cpp kiconeffect.cpp kicontheme.cpp \
#	kiconloader.cpp kwin.cpp kwinmodule.cpp krootprop.cpp kcharsets.cpp \
#	kckey.cpp kshortcut.cpp kkeynative_x11.cpp kkeyserver_x11.cpp \
#	kaccelaction.cpp kshortcutmenu.cpp kaccelbase.cpp kaccel.cpp \
#	kglobalaccel_x11.cpp kglobalaccel.cpp kstdaccel.cpp kshortcutlist.cpp \
#	kcrash.cpp kurl.cpp kglobal.cpp kglobalsettings.cpp \
#	kallocator.cpp kvmallocator.cpp kmimesourcefactory.cpp \
#	kinstance.cpp kpalette.cpp kipc.cpp klibloader.cpp ktempfile.cpp \
#	kuniqueapplication.cpp kacceleratormanager.cpp \
#	ksavefile.cpp krandomsequence.cpp kstringhandler.cpp kcompletion.cpp \
#	kcmdlineargs.cpp kaboutdata.cpp kcompletionbase.cpp knotifyclient.cpp \
#	kaudioplayer.cpp kdcoppropertyproxy.cpp \
#	netsupp.cpp kprocio.cpp krandom.cpp \
#	kpixmapprovider.cpp ktoolinvocation.cpp \
#	kcodecs.cpp ksocks.cpp fakes.c \
#	ksycoca.cpp ksycocadict.cpp ksycocafactory.cpp ksycoca.skel \
#	kxmessages.cpp kstartupinfo.cpp kcatalog.cpp \
#	kstaticdeleter.cpp kappdcopiface.cpp \
#	kappdcopiface.skel kclipboard.cpp kcheckaccelerators.cpp \
#	kdeversion.cpp kdebugdcopiface.cpp kdebugdcopiface.skel \
#	kcalendarsystem.cpp kcalendarsystemgregorian.cpp \
#	kcalendarsystemhijri.cpp kcalendarsystemhebrew.cpp \
#	kcalendarsystemfactory.cpp kmacroexpander.cpp kidna.cpp \
#	ktempdir.cpp kshell.cpp kmountpoint.cpp kcalendarsystemjalali.cpp \
#	kprotocolinfo_kdecore.cpp kprotocolinfofactory.cpp kxerrorhandler.cpp \
#	kuser.cpp kconfigskeleton.cpp klockfile.cpp \
#	ktimezones.cpp ktzfiletimezone.cpp kdatetime.cpp kmath.cpp \
#	kfilterbase.cpp kfilterdev.cpp kgzipfilter.cpp kbzip2filter.cpp \
#	kdedmodule.cpp kdedmodule.skel
#
#
#libkdecore_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(KDE_MT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) -version-info 6:0:2 -no-undefined
#libkdecore_la_LIBADD = malloc/libklmalloc.la $(SVGICON_LIB) ../dcop/libDCOP.la ../libltdl/libltdlc.la $(LIB_XEXT) $(LIBRESOLV) $(LIBUTIL) $(AGG_LIBS) $(LIB_IDN) network/libkdecorenetwork.la $(LIBZ) $(LIBBZ2) $(LIB_QT) $(LIB_QT3SUPPORT) @LTLIBINTL@ # ../kdefx/libkdefx.la
#libkdecore_la_NMCHECK = $(srcdir)/libkdecore.nmcheck
#libkdecore_la_NMCHECKWEAK = $(srcdir)/libkdecore_weak.nmcheck $(srcdir)/libqt-mt_weak.nmcheck \
#	$(top_srcdir)/dcop/libDCOP_weak.nmcheck $(top_srcdir)/kdecore/standard_weak.nmcheck
#
#libkdecore_la_METASOURCES = AUTO
#
#SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/kdecore
#
#kdebugdir = $(kde_confdir)
#kdebug_DATA = kdebug.areas kdebugrc language.codes
#
#.PHONY: kckey_h parser
#kckey_h : $(srcdir)/generate_keys.sh $(QNAMESPACE_H)
#	(cd $(srcdir) && ./generate_keys.sh $(QNAMESPACE_H))
#
## Also uninstall the old names of the KDE color palette files
#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(kde_locale)
#	$(INSTALL_DATA) $(srcdir)/all_languages.desktop $(DESTDIR)$(kde_locale)/all_languages
#	$(mkinstalldirs) $(DESTDIR)$(includedir)/kio
#	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/knotify
#	$(INSTALL_DATA) $(srcdir)/eventsrc $(DESTDIR)$(kde_datadir)/knotify/eventsrc
#	rm -f "$(DESTDIR)$(kde_confdir)/colors/40 Colors"
#	rm -f $(DESTDIR)$(kde_confdir)/colors/Web
#	rm -f $(DESTDIR)$(kde_confdir)/colors/Royal
#
## If you add a color palette file here, please change kdelibs/kdeui/kcolordialog.cpp too to allow to translate the name
#colors_DATA = 40.colors Web.colors Royal.colors Rainbow.colors
#colorsdir = $(kde_confdir)/colors
#
#EXTRA_DIST = generate_keys.sh $(kdebug_DATA) \
#	DESIGN.COLOR KCONFIG_DESIGN eventsrc \
#	kentities.c kentities.gperf makeentities \
#	all_languages.desktop
#
#kcharsets.lo: kentities.c
#
#parser: kentities.gperf
#	cd $(srcdir) && gperf -a -L "ANSI-C" -E -C -c -o -t -k '*' -Nkde_findEntity -D -Hhash_Entity -Wwordlist_Entity -s 2 kentities.gperf > kentities.c
#
#bin_PROGRAMS = kde-config kgrantpty
#
#kde_config_SOURCES = kde-config.cpp
#kde_config_LDADD = ./libkdecore.la
#kde_config_LDFLAGS = $(KDE_RPATH) $(KDE_MT_LDFLAGS)
#
#kgrantpty_SOURCES = kgrantpty.c
#kgrantpty_CFLAGS= $(KDE_USE_FPIE)
#kgrantpty_LDFLAGS = $(KDE_USE_PIE) $(KDE_RPATH) $(all_libraries)
#kgrantpty_LDADD = ./libkdefakes.la
#
## kgrantpty needs to be installed setuid root
#install-exec-hook:
#	@(chown 0 $(DESTDIR)$(bindir)/kgrantpty && chmod 4755 $(DESTDIR)$(bindir)/kgrantpty) || echo "Please make kgrantpty setuid root" >&2
#	@echo ""
#	@echo "kgrantpty is by default installed with a set SETUID root bit!"
#	@echo "This is needed for konsole, etc. to ensure that they can't be eavesdropped."
#	@echo ""
#
#DISTCLEANFILES = kde-config.cpp
#
#DOXYGEN_REFERENCES = dcop kdeui kparts kio
#DOXYGEN_EXCLUDE = malloc kde_file.h
#
#include ../admin/Doxyfile.am
