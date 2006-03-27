kde4_header()



########### install files ###############

install_files( ${MIME_INSTALL_DIR}/uri FILES mms.desktop mmst.desktop mmsu.desktop pnm.desktop rtspt.desktop rtspu.desktop )

kde4_footer()



#original Makefile.am contents follow:

#mimetypeuridata_DATA = mms.desktop mmst.desktop mmsu.desktop pnm.desktop rtspt.desktop rtspu.desktop
#mimetypeuridatadir = $(kde_mimedir)/uri
#
#EXTRA_DIST = $(mimetypeuridata_DATA)
#
