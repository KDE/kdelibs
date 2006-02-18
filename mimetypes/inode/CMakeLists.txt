kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${MIME_INSTALL_DIR}/inode FILES block.desktop chardevice.desktop directory-locked.desktop  	directory.desktop fifo.desktop socket.desktop )

kde4_footer()



#original Makefile.am contents follow:

#
#mimetypeinodedata_DATA = block.desktop chardevice.desktop directory-locked.desktop \
#	directory.desktop fifo.desktop socket.desktop
#
#mimetypeinodedatadir = $(kde_mimedir)/inode
#
#EXTRA_DIST = $(mimetypeinodedata_DATA)
