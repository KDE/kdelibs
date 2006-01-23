kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

#test5.cpp test5.h: $(srcdir)/test5.kcfg ../kconfig_compiler $(srcdir)/test5.kcfgc
#	../kconfig_compiler $(srcdir)/test5.kcfg $(srcdir)/test5.kcfgc

macro(GEN_KCFG_TEST_SOURCE _testName)
   add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_testName}.cpp
   COMMAND ${KDE4_KCFGC_EXECUTABLE} ARGS
   ${CMAKE_CURRENT_SOURCE_DIR}${_testName}.kcfg ${CMAKE_CURRENT_SOURCE_DIR}${_testName}.kcfgc
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}${_testName}.kcfg ${CMAKE_CURRENT_SOURCE_DIR}${_testName}.kcfgc)
endmacro(GEN_KCFG_TEST_SOURCE)

########### next target ###############

set(test1_SRCS
test1main.cpp
test1.cpp
)

kde4_automoc(${test1_SRCS})

if(KDE4_BUILD_TESTS)

gen_kcfg_test_source(test1)

kde4_add_executable(test1 ${test1_SRCS})

target_link_libraries(test1  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test2_SRCS
test2main.cpp
test2.cpp
)

kde4_automoc(${test2_SRCS})

if(KDE4_BUILD_TESTS)

gen_kcfg_test_source(test2)

kde4_add_executable(test2 ${test2_SRCS})

target_link_libraries(test2  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test3_SRCS
test3main.cpp
test3.cpp
)

kde4_automoc(${test3_SRCS})

if(KDE4_BUILD_TESTS)

gen_kcfg_test_source(test3)

kde4_add_executable(test3 ${test3_SRCS})

target_link_libraries(test3  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test4_SRCS
test4main.cpp
test4.cpp
)

kde4_automoc(${test4_SRCS})

if(KDE4_BUILD_TESTS)

gen_kcfg_test_source(test4)

kde4_add_executable(test4 ${test4_SRCS})

target_link_libraries(test4  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test5_SRCS
test5main.cpp
test5.cpp
)

kde4_automoc(${test5_SRCS})

if(KDE4_BUILD_TESTS)

gen_kcfg_test_source(test5)

kde4_add_executable(test5 ${test5_SRCS})

target_link_libraries(test5  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test6_SRCS
test6main.cpp
test6.cpp
)

kde4_automoc(${test6_SRCS})

if(KDE4_BUILD_TESTS)

gen_kcfg_test_source(test6)

kde4_add_executable(test6 ${test6_SRCS})

target_link_libraries(test6  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test7_SRCS
test7main.cpp
test7.cpp
)

kde4_automoc(${test7_SRCS})

if(KDE4_BUILD_TESTS)

gen_kcfg_test_source(test7)

kde4_add_executable(test7 ${test7_SRCS})

target_link_libraries(test7  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test8_SRCS
test8main.cpp
test8a.cpp
test8b.cpp
)

kde4_automoc(${test8_SRCS})

if(KDE4_BUILD_TESTS)

gen_kcfg_test_source(test8a)
gen_kcfg_test_source(test8b)

kde4_add_executable(test8 ${test8_SRCS})

target_link_libraries(test8  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test9_SRCS
test9main.cpp
test9.cpp
)

kde4_automoc(${test9_SRCS})

if(KDE4_BUILD_TESTS)

gen_kcfg_test_source(test9)

kde4_add_executable(test9 ${test9_SRCS})

target_link_libraries(test9  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(test_dpointer_SRCS
test_dpointer_main.cpp
test_dpointer.cpp
)

kde4_automoc(${test_dpointer_SRCS})

if(KDE4_BUILD_TESTS)

gen_kcfg_test_source(test_dpointer)

kde4_add_executable(test_dpointer ${test_dpointer_SRCS})

target_link_libraries(test_dpointer  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kconfigcompiler_test_SRCS
kconfigcompiler_test.cpp
)

kde4_automoc(${kconfigcompiler_test_SRCS})

if(KDE4_BUILD_TESTS)

#kde4_add_executable(kconfigcompiler_test ${kconfigcompiler_test_SRCS})

#target_link_libraries(kconfigcompiler_test  ${QT_AND_KDECORE_LIBS} QtTest_debug )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -I$(top_srcdir)/kdecore $(all_includes) -DQT_NO_CAST_ASCII -DSRCDIR=\"$(srcdir)\"
#
#METASOURCES = AUTO
#
#LDADD = $(LIB_KDECORE)
#
#check_PROGRAMS = test1 test2 test3 test4 test5 test6 test7 test8 test9 test_dpointer kconfigcompiler_test
#
#CLEANFILES = test1.cpp test1.h \
#             test2.cpp test2.h \
#             test3.cpp test3.h \
#             test4.cpp test4.h \
#             test5.cpp test5.h \
#             test6.cpp test6.h \
#             test7.cpp test7.h \
#             test8a.cpp test8a.h test8b.cpp test8b.h \
#             test9.cpp test9.h \
#						 test_dpointer.cpp test_dpointer.h \
#             md5sums
#
#test1_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#test1_SOURCES = test1main.cpp test1.cpp
#
#test2_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#test2_SOURCES = test2main.cpp test2.cpp
#
#test3_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#test3_SOURCES = test3main.cpp test3.cpp
#
#test4_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#test4_SOURCES = test4main.cpp test4.cpp
#
#test5_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#test5_SOURCES = test5main.cpp test5.cpp
#
#test6_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#test6_SOURCES = test6main.cpp test6.cpp
#
#test7_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#test7_SOURCES = test7main.cpp test7.cpp
#
#test8_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#test8_SOURCES = test8main.cpp test8a.cpp test8b.cpp
#
#test9_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#test9_SOURCES = test9main.cpp test9.cpp
#
#test_dpointer_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#test_dpointer_SOURCES = test_dpointer_main.cpp test_dpointer.cpp
#
#kconfigcompiler_test_SOURCES = kconfigcompiler_test.cpp
#kconfigcompiler_test_LDADD = $(LIB_KDECORE) -lQtTest_debug
#kconfigcompiler_test_LDFLAGS = $(all_libraries)
#
#TESTS = kconfigcompiler_test
#
#test1main.o test1.o: test1.h
## avoid running the below command in parallel
#test1.cpp: test1.h
#test1.cpp test1.h: $(srcdir)/test1.kcfg ../kconfig_compiler $(srcdir)/test1.kcfgc
#	../kconfig_compiler $(srcdir)/test1.kcfg $(srcdir)/test1.kcfgc
#
#test2main.o test2.o: test2.h
## avoid running the below command in parallel
#test2.cpp: test2.h
#test2.cpp test2.h: $(srcdir)/test2.kcfg ../kconfig_compiler $(srcdir)/test2.kcfgc
#	../kconfig_compiler $(srcdir)/test2.kcfg $(srcdir)/test2.kcfgc
#
#test3main.o test3.o: test3.h
## avoid running the below command in parallel
#test3.cpp: test3.h
#test3.cpp test3.h: $(srcdir)/test3.kcfg ../kconfig_compiler $(srcdir)/test3.kcfgc
#	../kconfig_compiler $(srcdir)/test3.kcfg $(srcdir)/test3.kcfgc
#
#test4main.o test4.o: test4.h
## avoid running the below command in parallel
#test4.cpp: test4.h
#test4.cpp test4.h: $(srcdir)/test4.kcfg ../kconfig_compiler $(srcdir)/test4.kcfgc
#	../kconfig_compiler $(srcdir)/test4.kcfg $(srcdir)/test4.kcfgc
#
#test5main.o test5.o: test5.h
## avoid running the below command in parallel
#test5.cpp: test5.h
#test5.cpp test5.h: $(srcdir)/test5.kcfg ../kconfig_compiler $(srcdir)/test5.kcfgc
#	../kconfig_compiler $(srcdir)/test5.kcfg $(srcdir)/test5.kcfgc
#
#test6main.o test6.o: test6.h
## avoid running the below command in parallel
#test6.cpp: test6.h
#test6.cpp test6.h: $(srcdir)/test6.kcfg ../kconfig_compiler $(srcdir)/test6.kcfgc
#	../kconfig_compiler $(srcdir)/test6.kcfg $(srcdir)/test6.kcfgc
#
#test7main.o test7.o: test7.h
## avoid running the below command in parallel
#test7.cpp: test7.h
#test7.cpp test7.h: $(srcdir)/test7.kcfg ../kconfig_compiler $(srcdir)/test7.kcfgc
#	../kconfig_compiler $(srcdir)/test7.kcfg $(srcdir)/test7.kcfgc
#
#test8main.o test8a.o test8b.o: test8a.h test8b.h
## avoid running the below command in parallel
#test8a.cpp: test8a.h
#test8a.cpp test8a.h: $(srcdir)/test8a.kcfg ../kconfig_compiler $(srcdir)/test8a.kcfgc
#	../kconfig_compiler $(srcdir)/test8a.kcfg $(srcdir)/test8a.kcfgc
#test8b.cpp: test8b.h
#test8b.cpp test8b.h: $(srcdir)/test8b.kcfg ../kconfig_compiler $(srcdir)/test8b.kcfgc
#	../kconfig_compiler $(srcdir)/test8b.kcfg $(srcdir)/test8b.kcfgc
#
#test9main.o test9.o: test9.h
## avoid running the below command in parallel
#test9.cpp: test9.h
#test9.cpp test9.h: $(srcdir)/test9.kcfg ../kconfig_compiler $(srcdir)/test9.kcfgc
#	../kconfig_compiler $(srcdir)/test9.kcfg $(srcdir)/test9.kcfgc
#
#test_dpointer_main.o test_dpointer.o: test_dpointer.h
## avoid running the below command in parallel
#test_dpointer.cpp: test_dpointer.h
#test_dpointer.cpp test_dpointer.h: $(srcdir)/test_dpointer.kcfg ../kconfig_compiler $(srcdir)/test_dpointer.kcfgc
#	../kconfig_compiler $(srcdir)/test_dpointer.kcfg $(srcdir)/test_dpointer.kcfgc
#
#md5sums:
#	$(MD5SUM) $(srcdir)/test*.ref | sed -e "s,$(srcdir)/,,; s,\.ref$$,," > md5sums
#
#md5check: test1.cpp test2.cpp test3.cpp test4.cpp test5.cpp test6.cpp test7.cpp test8a.cpp test8b.cpp test9.cpp md5sums
#	$(MD5SUM) -c md5sums
#
