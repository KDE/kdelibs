kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/LICENSES FILES BSD GPL_V2 LGPL_V2 QPL_V1.0 ARTISTIC )

kde4_footer()



#original Makefile.am contents follow:

#licensesdata_DATA = BSD GPL_V2 LGPL_V2 QPL_V1.0 ARTISTIC
#licensesdatadir = $(kde_datadir)/LICENSES
