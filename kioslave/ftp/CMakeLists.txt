kde4_header()

project(kioslave-file)

include_directories( ${CMAKE_SOURCE_DIR}/kdecore/network ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_ftp_PART_SRCS
ftp.cc
)

kde4_automoc(${kio_ftp_PART_SRCS})

kde4_add_plugin(kio_ftp ${kio_ftp_PART_SRCS})

target_link_libraries(kio_ftp  ${QT_AND_KDECORE_LIBS} kio )

install_targets(/lib/kde4 kio_ftp )


########### install files ###############

install_files( ${KDE4_SERVICES_DIR} FILES ftp.protocol )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= -I$(top_srcdir)/kdecore/network $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT 
######## Files
#
#kde_module_LTLIBRARIES = kio_ftp.la
#
#kio_ftp_la_SOURCES = ftp.cc
#kio_ftp_la_LIBADD  = $(LIB_KIO)
#kio_ftp_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) 
#
#noinst_HEADERS = ftp.h
#
#kdelnk_DATA = ftp.protocol
#kdelnkdir = $(kde_servicesdir)
#
#
