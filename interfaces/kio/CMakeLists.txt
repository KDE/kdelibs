kde4_header()

include_directories( ${QT_INCLUDES}  )


########### install files ###############

install_files( /include/kio FILES http.h )

kde4_footer()



#original Makefile.am contents follow:

#kioincludedir = $(includedir)/kio
#kioinclude_HEADERS = http.h
#
