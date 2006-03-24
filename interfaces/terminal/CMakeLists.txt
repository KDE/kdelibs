kde4_header()

include_directories( ${QT_INCLUDES}  )


########### install files ###############
install_files( /include FILES kde_terminal_interface.h )


kde4_footer()



#original Makefile.am contents follow:

#ktexteditorinclude_HEADERS = kde_terminal_interface.h
#ktexteditorincludedir = $(includedir)
