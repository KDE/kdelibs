kde4_header()

########### install files ###############
install_files( /include/khexedit FILES byteseditinterface.h valuecolumninterface.h charcolumninterface.h zoominterface.h clipboardinterface.h )

install_files( ${SERVICETYPES_INSTALL_DIR} FILES kbytesedit.desktop )

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
