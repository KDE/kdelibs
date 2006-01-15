kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_MIME_DIR}/video FILES mpeg.desktop quicktime.desktop x-flic.desktop  	x-msvideo.desktop x-mng.desktop x-ms-asf.desktop x-ms-wmv.desktop  	x-theora.desktop mp4.desktop x-ogm.desktop x-matroska.desktop  	vnd.rn-realvideo.desktop avi.desktop )

kde4_footer()



#original Makefile.am contents follow:

#mimetypevideodata_DATA = mpeg.desktop quicktime.desktop x-flic.desktop \
#	x-msvideo.desktop x-mng.desktop x-ms-asf.desktop x-ms-wmv.desktop \
#	x-theora.desktop mp4.desktop x-ogm.desktop x-matroska.desktop \
#	vnd.rn-realvideo.desktop avi.desktop
#
#mimetypevideodatadir = $(kde_mimedir)/video
#
#EXTRA_DIST = $(mimetypevideodata_DATA)
#
