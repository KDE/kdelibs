kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kabc ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kabcformat_binary_PART_SRCS
binaryformat.cpp
)

kde4_automoc(${kabcformat_binary_PART_SRCS})

kde4_add_plugin(kabcformat_binary ${kabcformat_binary_PART_SRCS})

target_link_libraries(kabcformat_binary  ${QT_AND_KDECORE_LIBS} kabc )

install_targets(/lib/kde4 kabcformat_binary )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kabc/formats FILES binary.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kabc -I$(top_builddir)/kabc $(all_includes)
#
#kde_module_LTLIBRARIES = kabcformat_binary.la
#
#kabcformat_binary_la_SOURCES = binaryformat.cpp
#kabcformat_binary_la_LDFLAGS =  -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) \
#                                -no-undefined
#kabcformat_binary_la_LIBADD  = $(top_builddir)/kabc/libkabc.la
#kabcformat_binary_la_COMPILE_FIRST = $(top_builddir)/kabc/addressee.h
#
## these are the headers for your project
#noinst_HEADERS = binaryformat.h
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kabcformat_binary.pot
#
#linkdir = $(kde_datadir)/kabc/formats
#link_DATA = binary.desktop
#EXTRA_DIST = $(link_DATA)
