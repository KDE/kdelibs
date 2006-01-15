kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_MIME_DIR}/uri FILES mms.desktop mmst.desktop mmsu.desktop pnm.desktop rtspt.desktop rtspu.desktop )

kde4_footer()



#original Makefile.am contents follow:

#mimetypeuridata_DATA = mms.desktop mmst.desktop mmsu.desktop pnm.desktop rtspt.desktop rtspu.desktop
#mimetypeuridatadir = $(kde_mimedir)/uri
#
#EXTRA_DIST = $(mimetypeuridata_DATA)
#
