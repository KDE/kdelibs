kde4_header()



########### install files ###############

install_files( ${MIME_INSTALL_DIR}/multipart FILES mixed.desktop x-mixed-replace.desktop )

kde4_footer()



#original Makefile.am contents follow:

#mimetypemultipartdata_DATA = 	mixed.desktop x-mixed-replace.desktop
#
#mimetypemultipartdatadir = $(kde_mimedir)/multipart
#
#EXTRA_DIST = $(mimetypemultipartdata_DATA)
#
