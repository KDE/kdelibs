kde4_header()



########### install files ###############

install_files( ${MIME_INSTALL_DIR}/video FILES mpeg.desktop quicktime.desktop x-flic.desktop  	x-msvideo.desktop x-mng.desktop x-ms-asf.desktop x-ms-wmv.desktop  	x-theora.desktop mp4.desktop x-ogm.desktop x-matroska.desktop  	vnd.rn-realvideo.desktop avi.desktop )

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
