kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kio/kio ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_SRCS
fileprops.cpp
)

kde4_automoc(${kfile_SRCS})

kde4_add_executable(kfile ${kfile_SRCS})

target_link_libraries(kfile  ${QT_AND_KDECORE_LIBS} kio )

install_targets(/bin kfile )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_HEADERS = fileprops.h
#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kio/kio $(all_includes)
#
#bin_PROGRAMS = kfile
#kfile_SOURCES = fileprops.cpp
#kfile_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kfile_LDADD = $(LIB_KIO)
#
