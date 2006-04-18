kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### next target ###############

set(kcm_crypto_PART_SRCS crypto.cpp certexport.cpp kdatetimedlg.cpp )

kde4_automoc(${kcm_crypto_PART_SRCS})

kde4_add_plugin(kcm_crypto ${kcm_crypto_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_crypto )

target_link_libraries(kcm_crypto  ${KDE4_KDE3SUPPORT_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} kcm_crypto )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES crypto.desktop )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = $(all_includes) $(SSL_INCLUDES)
#kde_module_LTLIBRARIES = kcm_crypto.la
#
#kcm_crypto_la_SOURCES = crypto.cpp certexport.cpp kdatetimedlg.cpp
#
#kcm_crypto_la_LDFLAGS  = $(KDE_RPATH) $(all_libraries) $(SSL_LDFLAGS) -module -avoid-version -no-undefined
#kcm_crypto_la_LIBADD = -lkdeui $(LIB_KIO) $(LIBSSL)
#
#METASOURCES = AUTO
#
#noinst_HEADERS = crypto.h certexport.h kdatetimedlg.h
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmcrypto.pot
#
#xdg_apps_DATA = crypto.desktop
