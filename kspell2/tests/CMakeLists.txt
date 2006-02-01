kde4_header()

if(KDE4_BUILD_TESTS)
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/kspell2/ui )


########### next target ###############

set(test_SRCS
test.cpp
)

kde4_automoc(${test_SRCS})

kde4_add_executable(test ${test_SRCS})

target_link_libraries(test  ${QT_AND_KDECORE_LIBS} kspell2 )


########### next target ###############

set(test_filter_SRCS
test_filter.cpp
)

kde4_automoc(${test_filter_SRCS})

kde4_add_executable(test_filter ${test_filter_SRCS})

target_link_libraries(test_filter  ${QT_AND_KDECORE_LIBS} kspell2 )


########### next target ###############

set(backgroundtest_SRCS
backgroundtest.cpp
)

kde4_automoc(${backgroundtest_SRCS})

kde4_add_executable(backgroundtest ${backgroundtest_SRCS})

target_link_libraries(backgroundtest  ${QT_AND_KDECORE_LIBS} kspell2 )


########### next target ###############

set(test_dialog_SRCS
test_dialog.cpp
)

kde4_automoc(${test_dialog_SRCS})

kde4_add_executable(test_dialog ${test_dialog_SRCS})

target_link_libraries(test_dialog  ${QT_AND_KDECORE_LIBS} kspell2 )


########### next target ###############

set(test_highlighter_SRCS
test_highlighter.cpp
${kspell2_noinst_STAT_SRCS}
)

kde4_automoc(${test_highlighter_SRCS})


message(STATUS "skipping kabc/tests test_highlighter")

#kde4_add_executable(test_highlighter ${test_highlighter_SRCS})
#
#target_link_libraries(test_highlighter  ${QT_AND_KDECORE_LIBS} kspell2base )


########### next target ###############

set(test_configdialog_SRCS
test_configdialog.cpp
${kspell2_noinst_STAT_SRCS}
)




message(STATUS "skipping kabc/tests test_configdialog")

#kde4_automoc(${test_configdialog_SRCS})
#
#kde4_add_ui3_files(kspell2_LIB_SRCS  ${CMAKE_SOURCE_DIR}/kspell2/ui/kspell2ui.ui ${CMAKE_SOURCE_DIR}/kspell2/ui/configui.ui)
#
#kde4_add_executable(test_configdialog ${test_configdialog_SRCS})
#
#target_link_libraries(test_configdialog  ${QT_AND_KDECORE_LIBS} kspell2base )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -I$(top_srcdir)/kspell2 -I$(top_srcdir)/kspell2/ui $(all_includes)
#METASOURCES = AUTO
#
#
#check_PROGRAMS = test test_filter backgroundtest \
#	test_dialog test_highlighter test_configdialog
#
#test_SOURCES = test.cpp
#test_LDADD = ../ui/libkspell2.la $(LIB_KDECORE)
#
#test_filter_SOURCES = test_filter.cpp
#test_filter_LDADD = ../ui/libkspell2.la $(LIB_KDECORE)
#
#backgroundtest_SOURCES = backgroundtest.cpp
#backgroundtest_LDADD = ../ui/libkspell2.la $(LIB_KDECORE)
#
#test_dialog_SOURCES = test_dialog.cpp
#test_dialog_LDADD = ../ui/libkspell2.la $(LIB_KDECORE)
#
#test_highlighter_SOURCES = test_highlighter.cpp
#test_highlighter_LDADD = ../ui/libkspell2_noinst.la ../libkspell2base.la $(LIB_KDECORE)
#
#test_config_SOURCES = test_config.cpp
#test_config_LDADD = ../ui/libkspell2_noinst.la ../libkspell2base.la $(LIB_KDECORE)
#
#test_configdialog_SOURCES = test_configdialog.cpp
#test_configdialog_LDADD = ../ui/libkspell2_noinst.la ../libkspell2base.la $(LIB_KDECORE)
