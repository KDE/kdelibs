kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kio ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_SERVICESTYPES_DIR} FILES kregexpeditor.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kio -I$(top_srcdir) $(all_includes)
#
#include_HEADERS = kregexpeditorinterface.h
#
#METASOURCES = AUTO
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kregexpeditor.desktop
