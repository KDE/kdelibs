kde4_header()

if(KDE4_BUILD_TESTS)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_SOURCE_DIR} )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(kdehwtest_SRCS 
   kdehwtest.cpp 
   fakedevice.cpp 
   fakemanager.cpp 
   fakeprocessor.cpp )

kde4_automoc(${kdehwtest_SRCS})

kde4_add_executable(kdehwtest ${kdehwtest_SRCS})

target_link_libraries(kdehwtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kdehw kdehwifaces)

add_test(kdehwtest ${EXECUTABLE_OUTPUT_PATH}/kdehwtest)

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#check_PROGRAMS = kdehwtest
#
#TESTS = kdehwtest
#
#INCLUDES = -I$(srcdir)/../kdehw $(all_includes)
#AM_LDFLAGS = $(all_libraries)
#
#LDFLAGS = $(KDE_RPATH) $(all_libraries)
#LDADD = $(LIB_KDECORE) -lQtTest_debug ../kdehw/libkdehw.la ../kdehw/ifaces/libkdehwifaces.la
#
#kdehwtest_SOURCES = kdehwtest.cpp fakedevice.cpp fakemanager.cpp fakeprocessor.cpp
#
#METASOURCES = AUTO
#
