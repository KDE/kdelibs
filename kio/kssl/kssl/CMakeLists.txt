kde4_header()


add_subdirectory( caroot )
########### install files ###############

install_files( ${CONFIG_INSTALL_DIR} FILES ksslcalist )

kde4_footer()



#original Makefile.am contents follow:

#
#SUBDIRS = caroot
#
#confdir = $(kde_confdir)
#conf_DATA = ksslcalist
#
