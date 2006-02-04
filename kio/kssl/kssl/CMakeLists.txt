kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

add_subdirectory( caroot )
########### install files ###############

install_files( ${KDE4_CONFIG_DIR} FILES ksslcalist )

kde4_footer()



#original Makefile.am contents follow:

#
#SUBDIRS = caroot
#
#confdir = $(kde_confdir)
#conf_DATA = ksslcalist
#
