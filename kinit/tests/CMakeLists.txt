kde4_header()

if(KDE4_BUILD_TESTS)

   include_directories( ${KDE4_KIO_INCLUDES} )
   
   set(klaunchertest_SRCS klaunchertest.cpp )
   
   kde4_automoc(${klaunchertest_SRCS})
   
   kde4_add_executable(klaunchertest ${klaunchertest_SRCS})
   
   target_link_libraries(klaunchertest  ${KDE4_KDECORE_LIBS} kio )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES=	-I$(srcdir)/../.. $(all_includes) 
######## Files
#
#check_PROGRAMS = klaunchertest
#
#klaunchertest_SOURCES = klaunchertest.cpp 
#klaunchertest_LDADD   = $(LIB_KIO)
#
