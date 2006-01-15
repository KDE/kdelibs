kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_MIME_DIR}/audio FILES x-wav.desktop x-mod.desktop x-midi.desktop  	x-mp3.desktop x-mp2.desktop mp4.desktop mpeg.desktop  	basic.desktop x-adpcm.desktop x-aiff.desktop  	x-pn-realaudio.desktop vnd.rn-realaudio.desktop  	x-mpegurl.desktop mpegurl.desktop x-scpls.desktop  	vorbis.desktop x-vorbis.desktop  	x-flac.desktop x-oggflac.desktop x-speex.desktop  	x-matroska.desktop x-musepack.desktop  	ac3.desktop aac.desktop x-ms-wma.desktop prs.sid.desktop )

kde4_footer()



#original Makefile.am contents follow:

#mimetypeaudiodata_DATA = x-wav.desktop x-mod.desktop x-midi.desktop \
#	x-mp3.desktop x-mp2.desktop mp4.desktop mpeg.desktop \
#	basic.desktop x-adpcm.desktop x-aiff.desktop \
#	x-pn-realaudio.desktop vnd.rn-realaudio.desktop \
#	x-mpegurl.desktop mpegurl.desktop x-scpls.desktop \
#	vorbis.desktop x-vorbis.desktop \
#	x-flac.desktop x-oggflac.desktop x-speex.desktop \
#	x-matroska.desktop x-musepack.desktop \
#	ac3.desktop aac.desktop x-ms-wma.desktop prs.sid.desktop
#
#mimetypeaudiodatadir = $(kde_mimedir)/audio
#
#EXTRA_DIST = $(mimetypeaudiodata_DATA)
#
