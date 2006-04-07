kde4_header()



########### next target ###############



if(KDE4_BUILD_TESTS)

   set(test_SRCS main.cc )
   kde4_automoc(${test_SRCS})
   kde4_add_executable(test RUN_UNINSTALLED ${test_SRCS})
   target_link_libraries(test  ${KDE4_KDECORE_LIBS} kdeui kio )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES=$(all_includes)
#check_PROGRAMS=test
#test_SOURCES=main.cc
#test_LDADD=$(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) -lkio
#test_LDFLAGS=$(all_libraries)
#METASOURCES=AUTO
