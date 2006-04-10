kde4_header()

########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES kregexpeditor.desktop )
install_files( /include FILES kregexpeditorinterface.h )

kde4_footer()



#original Makefile.am contents follow:

#include_HEADERS = kregexpeditorinterface.h
#
#METASOURCES = AUTO
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kregexpeditor.desktop
