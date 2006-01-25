kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_SERVICETYPES_DIR} FILES dcopinstantmessenger.desktop )
install_files( ${KDE4_DATA_DIR}/kcm_componentchooser FILES kcm_instantmessenger.desktop )
install_files( /include FILES kimiface.h )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#kimifaceinclude_HEADERS = kimiface.h
#kimifaceincludedir = $(includedir)
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = dcopinstantmessenger.desktop
#
#kcm_instantmessenger_DATA = kcm_instantmessenger.desktop
#kcm_instantmessengerdir = $(kde_datadir)/kcm_componentchooser
