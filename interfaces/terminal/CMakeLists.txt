kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############
install_files( /include FILES kde_terminal_interface.h )


kde4_footer()



#original Makefile.am contents follow:

#ktexteditorinclude_HEADERS = kde_terminal_interface.h
#ktexteditorincludedir = $(includedir)
