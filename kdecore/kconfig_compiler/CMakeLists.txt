kde4_header()

add_subdirectory( tests ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kconfig_compiler_SRCS
kconfig_compiler.cpp
)

kde4_automoc(${kconfig_compiler_SRCS})

kde4_add_executable(kconfig_compiler ${kconfig_compiler_SRCS})

target_link_libraries(kconfig_compiler  ${QT_AND_KDECORE_LIBS} ${QT_QTXML_LIBRARY} )

install_targets(/bin kconfig_compiler )


########### next target ###############

#set(example_SRCS
#example.cpp
#exampleprefs_base.cpp
#)
#
#kde4_automoc(${example_SRCS})

if(KDE4_BUILD_TESTS)

message(STATUS "Skipped kdecore/kconfig_compiler example")

#kde4_add_executable(example ${example_SRCS})
#
#target_link_libraries(example  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

#set(autoexample_SRCS
#exampleprefs_base.cpp
#autoexample.cpp
#)
#
#kde4_automoc(${autoexample_SRCS})
#
#set( autoexample_UI3
#general_base.ui3
#myoptions_base.ui3
#)
#
#kde4_add_ui3_files(autoexample_SRCS ${autoexample_UI3} )
#
if(KDE4_BUILD_TESTS)
message(STATUS "Skipped kdecore/kconfig_compiler autotest")
#kde4_add_executable(autoexample ${autoexample_SRCS})
#
#target_link_libraries(autoexample  ${QT_AND_KDECORE_LIBS} kdeui )
#
endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = tests
#
#AM_CPPFLAGS = -I$(top_srcdir)/kdecore -I$(top_srcdir) $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#bin_PROGRAMS = kconfig_compiler
#
#kconfig_compiler_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kconfig_compiler_LDADD = $(LIB_KDECORE)
#kconfig_compiler_SOURCES = kconfig_compiler.cpp
#
#check_PROGRAMS = example # autoexample
#EXTRA_PROGRAMS = autoexample
#
#example_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#example_LDADD = $(LIB_KDECORE)
#example_SOURCES = example.cpp exampleprefs_base.cpp
#
#autoexample_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#autoexample_LDADD = $(LIB_KDECORE) $(LIB_KDEUI)
#autoexample_SOURCES = exampleprefs_base.cpp general_base.ui3 myoptions_base.ui3 \
#  autoexample.cpp  
#
#example.o exampleprefs_base.o: exampleprefs_base.h
## avoid running the below command in parallel
#exampleprefs_base.cpp: exampleprefs_base.h
#exampleprefs_base.cpp exampleprefs_base.h: $(srcdir)/example.kcfg kconfig_compiler $(srcdir)/exampleprefs_base.kcfgc
#	./kconfig_compiler $(srcdir)/example.kcfg $(srcdir)/exampleprefs_base.kcfgc
#
#METASOURCES = AUTO
#
#CLEANFILES = exampleprefs_base.h exampleprefs_base.cpp
#
#TESTFILES = test1.kcfg test2.kcfg test3.kcfg test4.kcfg test_dpointer.kcfg
#
#check-local:
#	for i in $(TESTFILES); \
#	  do xmllint --noout --schema $(srcdir)/kcfg.xsd $(srcdir)/tests/$$i; \
#	  perl $(top_srcdir)/kdecore/kconfig_compiler/checkkcfg.pl \
#          $(top_srcdir)/kdecore/kconfig_compiler/tests/$$i; done
#	  
