kde4_header()

########### next target ###############

set(kfile_SRCS fileprops.cpp )

kde4_automoc(${kfile_SRCS})

kde4_add_executable(kfile NOGUI ${kfile_SRCS})

target_link_libraries(kfile  ${KDE4_KDECORE_LIBS} kio )

install(TARGETS kfile DESTINATION bin)


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
