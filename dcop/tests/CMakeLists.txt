kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(dcop_test_SRCS
test.cpp
)

kde4_automoc(${dcop_test_SRCS})

set( dcop_test_DCOP_SKEL_SRCS
test.h
)

kde4_add_dcop_skels(dcop_test_SRCS ${dcop_test_DCOP_SKEL_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(dcop_test ${dcop_test_SRCS})

target_link_libraries(dcop_test  ${QT_AND_KDECORE_LIBS} DCOP )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(driver_SRCS
driver.cpp
)

kde4_automoc(${driver_SRCS})

set( driver_DCOP_STUB_SRCS
test.h
)

kde4_add_dcop_stubs(driver_SRCS ${driver_DCOP_STUB_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(driver ${driver_SRCS})

target_link_libraries(driver  ${QT_AND_KDECORE_LIBS} DCOP )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#nothing:
#	@echo -e "dcop tests not run.\nRun make tests in the tests directory to make them"
#
#INCLUDES = $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#AM_LDFLAGS   = $(all_libraries)
#
#METASOURCES = AUTO
#
######## Files
#
#check_PROGRAMS = dcop_test driver
#
#dcop_test_SOURCES = test.cpp test.skel
#dcop_test_LDADD = ../libDCOP.la
#dcop_test_LDFLAGS = $(KDE_RPATH)
#
#driver_SOURCES = driver.cpp test.stub
#driver_LDADD = ../libDCOP.la
#driver_LDFLAGS = $(KDE_RPATH)
#
#CLEANFILES = batch.generated definitions.generated driver.generated shell.generated
#test.o: batch.generated definitions.generated driver.generated shell.generated
#
#test-clean:
#	rm batch.returns driver.returns shell.returns batch.stdout driver.stdout shell.stdout
#
#test.h: shell.generated
#batch.generated: shell.generated
#definitions.generated: shell.generated
#driver.generated: shell.generated
#
#shell.generated: $(srcdir)/testcases $(srcdir)/generate.pl
#	perl $(srcdir)/generate.pl < $(srcdir)/testcases
#
#check-local: dcop_test driver ../client/dcop
#	sh $(srcdir)/run-tests.sh
