kde4_header()



########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kssl FILES ca-bundle.crt )

kde4_footer()



#original Makefile.am contents follow:

#
#kssldatadir = $(kde_datadir)/kssl
#kssldata_DATA = ca-bundle.crt
#
