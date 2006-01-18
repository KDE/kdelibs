kde4_header()

add_subdirectory( kab ) 
add_subdirectory( vcardparser ) 
add_subdirectory( formats ) 
add_subdirectory( plugins ) 
add_subdirectory( scripts ) 
add_subdirectory( tests ) 

include_directories( ${CMAKE_SOURCE_DIR}/kabc ${CMAKE_SOURCE_DIR}/kabc/kab ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/addressee.cpp
  COMMAND ${PERL_EXECUTABLE}
  ARGS  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/makeaddressee
  DEPENDS  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/makeaddressee
   ${CMAKE_CURRENT_SOURCE_DIR}/scripts/addressee.src.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scripts/addressee.src.h
   ${CMAKE_CURRENT_SOURCE_DIR}/scripts/entrylist
   ${CMAKE_CURRENT_SOURCE_DIR}/scripts/field.src.cpp
  )

add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/field.cpp
  COMMAND ${PERL_EXECUTABLE}
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/makeaddressee
  DEPENDS  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/makeaddressee
   ${CMAKE_CURRENT_SOURCE_DIR}/scripts/addressee.src.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/scripts/addressee.src.h
   ${CMAKE_CURRENT_SOURCE_DIR}/scripts/entrylist
   ${CMAKE_CURRENT_SOURCE_DIR}/scripts/field.src.cpp
  )

########### next target ###############

set(kabc_LIB_SRCS
address.cpp
addressbook.cpp
addressee.cpp
addresseedialog.cpp
distributionlist.cpp
distributionlistdialog.cpp
distributionlisteditor.cpp
errorhandler.cpp
field.cpp
formatfactory.cpp
geo.cpp
key.cpp
phonenumber.cpp
picture.cpp
plugin.cpp
resource.cpp
secrecy.cpp
sound.cpp
stdaddressbook.cpp
vcardconverter.cpp
timezone.cpp
vcardformat.cpp
ldifconverter.cpp
addresslineedit.cpp
ldapclient.cpp
addresseelist.cpp
vcardtool.cpp
addresseehelper.cpp
lock.cpp
locknull.cpp
ldif.cpp
ldapurl.cpp
ldapconfigwidget.cpp
sortmode.cpp
)

kde4_automoc(${kabc_LIB_SRCS})

set( kabc_DCOP_SKEL_SRCS
addresseehelper.h
)

kde4_add_dcop_skels(kabc_LIB_SRCS ${kabc_DCOP_SKEL_SRCS})

kde4_add_library(kabc SHARED ${kabc_LIB_SRCS})

target_link_libraries(kabc  ${QT_AND_KDECORE_LIBS} vcards kio kresources )

set_target_properties(kabc PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install_targets(/lib kabc )


########### next target ###############

set(kab2kabc_SRCS
kab2kabc.cpp
)

kde4_automoc(${kab2kabc_SRCS})

kde4_add_executable(kab2kabc ${kab2kabc_SRCS})

target_link_libraries(kab2kabc  ${QT_AND_KDECORE_LIBS} kab )

install_targets(/bin kab2kabc )


########### install files ###############

install_files( /share/autostart FILES kab2kabc.desktop )
install_files( ${KDE4_DATA_DIR}/kabc FILES countrytransl.map )
install_files( ${KDE4_SERVICES_DIR}/kresources FILES kabc_manager.desktop )

kde4_create_doxygen_docs( REFERENCES kdecore kdeui EXCLUDE scripts  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = kab vcardparser . formats plugins scripts tests
#
## Make sure $(all_includes) remains last!
#INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc -I$(top_srcdir)/kabc/kab \
#           -I$(srcdir)/vcardparser $(all_includes)
#
#field.cpp: addressee.h addressee.cpp
#addressee.cpp: addressee.h
#addressee.cpp addressee.h field.cpp: \
#				$(srcdir)/scripts/makeaddressee \
#				$(srcdir)/scripts/addressee.src.cpp \
#				$(srcdir)/scripts/addressee.src.h \
#				$(srcdir)/scripts/entrylist \
#				$(srcdir)/scripts/field.src.cpp
#	mysrcdir=`cd $(srcdir)/scripts && pwd` ;\
#	cd scripts && $(PERL) $$mysrcdir/makeaddressee
#
#CLEANFILES = addressee.h addressee.cpp field.cpp
#
#lib_LTLIBRARIES = libkabc.la
#libkabc_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 3:0:2 -no-undefined
#libkabc_la_LIBADD = vcardparser/libvcards.la $(LIB_KIO) \
#                    $(top_builddir)/kresources/libkresources.la
#libkabc_la_COMPILE_FIRST = addressee.h
#
#libkabc_la_SOURCES = \
#  address.cpp addressbook.cpp addressee.cpp addresseedialog.cpp \
#  distributionlist.cpp distributionlistdialog.cpp distributionlisteditor.cpp \
#  errorhandler.cpp field.cpp formatfactory.cpp geo.cpp key.cpp \
#  phonenumber.cpp picture.cpp plugin.cpp resource.cpp \
#  secrecy.cpp sound.cpp stdaddressbook.cpp \
#  vcardconverter.cpp timezone.cpp \
#  vcardformat.cpp ldifconverter.cpp addresslineedit.cpp \
#  ldapclient.cpp addresseelist.cpp vcardtool.cpp addresseehelper.cpp \
#  addresseehelper.skel lock.cpp locknull.cpp ldif.cpp ldapurl.cpp ldapconfigwidget.cpp \
#  sortmode.cpp
#
#
#kabcincludedir = $(includedir)/kabc
#kabcinclude_HEADERS = address.h addressbook.h addressee.h addresseedialog.h \
#  distributionlist.h distributionlistdialog.h distributionlisteditor.h \
#  errorhandler.h field.h format.h formatfactory.h format.h geo.h key.h \
#  phonenumber.h picture.h plugin.h resource.h \
#  secrecy.h sound.h stdaddressbook.h timezone.h \
#  vcardformat.h vcardconverter.h ldifconverter.h \
#  addresslineedit.h ldapclient.h addresseelist.h lock.h locknull.h ldif.h \
#  ldapurl.h ldapconfigwidget.h sortmode.h
#
#METASOURCES = AUTO
#
#bin_PROGRAMS = kab2kabc
#
#kab2kabc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kab2kabc_LDADD = libkabc.la ./kab/libkab.la
#kab2kabc_SOURCES = kab2kabc.cpp
#
#autostart_DATA = kab2kabc.desktop
#autostartdir = $(datadir)/autostart
#
#manager_DATA = kabc_manager.desktop
#managerdir = $(kde_servicesdir)/kresources
#
#DOXYGEN_REFERENCES = kdecore kdeui
#DOXYGEN_EXCLUDE = scripts
#
#map_DATA = countrytransl.map
#mapdir = $(kde_datadir)/kabc
#
#EXTRA_DIST = $(map_DATA)
#
#include ../admin/Doxyfile.am
