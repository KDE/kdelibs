kde4_header()

include_directories( ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


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
