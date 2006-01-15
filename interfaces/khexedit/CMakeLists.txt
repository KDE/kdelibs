kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kparts ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_SERVICESTYPES_DIR} FILES kbytesedit.desktop )

kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES= -I$(top_srcdir)/kparts $(all_includes)
#
## the service type
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kbytesedit.desktop
#
## the interfaces
#khexeditinclude_HEADERS = byteseditinterface.h \
#                          valuecolumninterface.h charcolumninterface.h \
#                          zoominterface.h clipboardinterface.h
#
#khexeditincludedir = $(includedir)/khexedit
