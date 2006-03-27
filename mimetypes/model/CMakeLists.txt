kde4_header()



########### install files ###############

install_files( ${MIME_INSTALL_DIR}/model FILES vrml.desktop )

kde4_footer()



#original Makefile.am contents follow:

#mimetypemessagedata_DATA = vrml.desktop
#
#mimetypemessagedatadir = $(kde_mimedir)/model
#
#EXTRA_DIST = $(mimetypemessagedata_DATA)
#
