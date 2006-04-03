kde4_header()

add_subdirectory( ui ) 

########### install files ###############

install_files( /include/phonon/ifaces FILES
	abstractmediaproducer.h
	mediaobject.h
	soundcardcapture.h
	audiopath.h
	backend.h
	abstractaudiooutput.h
	audiooutput.h
	audiodataoutput.h
	videopath.h
	abstractvideooutput.h
	videoeffect.h
	audioeffect.h
	bytestream.h
	base.h
	volumefadereffect.h
	avcapture.h
	)

kde4_footer()
