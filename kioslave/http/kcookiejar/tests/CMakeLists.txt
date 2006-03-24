kde4_header()

if(KDE4_BUILD_TESTS)

########### next target ###############

set(kcookiejartest_SRCS kcookiejartest.cpp )

kde4_automoc(${kcookiejartest_SRCS})

kde4_add_executable(kcookiejartest ${kcookiejartest_SRCS})

target_link_libraries(kcookiejartest  ${KDE4_KDECORE_LIBS} kio )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES=	$(all_includes) 
#
######## Files
#
#check_PROGRAMS = kcookiejartest
#
#kcookiejartest_SOURCES = kcookiejartest.cpp 
#kcookiejartest_LDADD   = $(LIB_KIO)
#kcookiejartest_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#check-local: kcookiejartest
#	./kcookiejartest $(srcdir)/cookie.test
#	./kcookiejartest $(srcdir)/cookie_rfc.test
#	./kcookiejartest $(srcdir)/cookie_saving.test
#	./kcookiejartest $(srcdir)/cookie_settings.test
