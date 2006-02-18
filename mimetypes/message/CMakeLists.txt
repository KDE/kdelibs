kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${MIME_INSTALL_DIR}/message FILES rfc822.desktop news.desktop )

kde4_footer()



#original Makefile.am contents follow:

#mimetypemessagedata_DATA = 	rfc822.desktop news.desktop
#
#mimetypemessagedatadir = $(kde_mimedir)/message
#
#EXTRA_DIST = $(mimetypemessagedata_DATA)
#
