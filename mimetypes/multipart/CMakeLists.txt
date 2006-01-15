kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_MIME_DIR}/multipart FILES mixed.desktop x-mixed-replace.desktop )

kde4_footer()



#original Makefile.am contents follow:

#mimetypemultipartdata_DATA = 	mixed.desktop x-mixed-replace.desktop
#
#mimetypemultipartdatadir = $(kde_mimedir)/multipart
#
#EXTRA_DIST = $(mimetypemultipartdata_DATA)
#
