kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/khtml ${CMAKE_SOURCE_DIR}/kio/httpfilter ${CMAKE_SOURCE_DIR}/kutils ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kmultipart_PART_SRCS
kmultipart.cpp
)

kde4_automoc(${kmultipart_PART_SRCS})

kde4_add_plugin(kmultipart WITH_PREFIX ${kmultipart_PART_SRCS})

target_link_libraries(kmultipart  ${QT_AND_KDECORE_LIBS} kparts httpfilter )

install_targets(/lib/kde4 kmultipart )


########### install files ###############

install_files( ${KDE4_SERVICES_DIR} FILES kmultipart.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/khtml -I$(top_srcdir)/kio/httpfilter -I$(top_srcdir)/kutils $(all_includes)
#
## These are not really libraries, but modules dynamically opened.
## So they should be installed in kde_module_dir, which is usually $kde_prefix/lib/kde3
#kde_module_LTLIBRARIES = libkmultipart.la
#
#libkmultipart_la_SOURCES = kmultipart.cpp
#libkmultipart_la_LIBADD = $(LIB_KPARTS) $(top_builddir)/kio/httpfilter/libhttpfilter.la
#libkmultipart_la_DEPENDENCIES = $(LIB_KPARTS)
#libkmultipart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#
## Automatically generate moc files
#METASOURCES = AUTO
#
## Install the .desktop file into the kde_services directory
#kde_services_DATA = kmultipart.desktop
#
