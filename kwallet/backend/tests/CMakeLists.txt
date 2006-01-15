kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kwallet/backend ${CMAKE_SOURCE_DIR}/kwallet/client ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(backendtest_SRCS
backendtest.cpp
)

kde4_automoc(${backendtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(backendtest ${backendtest_SRCS})

target_link_libraries(backendtest  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testbf_SRCS
testbf.cpp
)

kde4_automoc(${testbf_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(testbf ${testbf_SRCS})

target_link_libraries(testbf  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testsha_SRCS
testsha.cpp
)

kde4_automoc(${testsha_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(testsha ${testsha_SRCS})

target_link_libraries(testsha  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kwallet/backend -I$(top_srcdir)/kwallet/client $(all_includes)
#
#AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
#
#check_PROGRAMS = backendtest testbf testsha
#
#METASOURCES = AUTO
#
#LDADD = ../libkwalletbackend.la ../../client/libkwalletclient.la
#backendtest_SOURCES = backendtest.cpp
#
#testbf_SOURCES = testbf.cpp
#
#testsha_SOURCES = testsha.cpp
