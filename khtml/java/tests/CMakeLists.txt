kde4_header()

if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_SOURCE_DIR}/khtml/java ${CMAKE_SOURCE_DIR}/khtml ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(testserver_SRCS
testkjavaappletserver.cpp
)

kde4_automoc(${testserver_SRCS})

kde4_add_executable(testserver ${testserver_SRCS})

target_link_libraries(testserver  ${QT_AND_KDECORE_LIBS} kjava khtml)

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/khtml/java -I$(top_srcdir)/khtml \
#           -I$(top_srcdir) $(all_includes)
#
#check_PROGRAMS = testserver 
#
#testserver_SOURCES = testkjavaappletserver.cpp
#testserver_LDADD = ../libkjava.la 
#
#METASOURCES = AUTO
