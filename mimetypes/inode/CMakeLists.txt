kde4_header()



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
