


########### next target ###############

set(kspell2_LIB_SRCS
 dialog.cpp
 ${kspell2base_STAT_SRCS}
 configwidget.cpp
 highlighter.cpp
 configdialog.cpp
)

kde4_automoc(${kspell2_LIB_SRCS})

kde4_add_ui_files(kspell2_LIB_SRCS kspell2ui.ui configui.ui)

kde4_add_library(kspell2 SHARED ${kspell2_LIB_SRCS})

target_link_libraries(kspell2  ${KDE4_KDECORE_LIBS} kdeui kio kutils)

set_target_properties(kspell2 PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install_targets(${LIB_INSTALL_DIR} kspell2 )


########### next target ###############


########### install files ###############
install_files( /include/kspell2 FILES dialog.h highlighter.h configdialog.h   configwidget.h )





#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#lib_LTLIBRARIES = libkspell2.la
## Put most of the code in a noinst lib, for the unit tests to be able to use internal classes.
#noinst_LTLIBRARIES = libkspell2_noinst.la
#
#KDE_CXXFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT
#INCLUDES = -I$(top_srcdir)/kspell2 $(all_includes)
#
#kspell2includedir = $(includedir)/kspell2
#kspell2include_HEADERS = dialog.h highlighter.h configdialog.h \
#	configwidget.h
#
#libkspell2_noinst_la_SOURCES = kspell2ui.ui configwidget.cpp \
#	highlighter.cpp configui.ui configdialog.cpp
#
#dialog.lo: kspell2ui.h
#
#libkspell2_la_SOURCES = dialog.cpp
#libkspell2_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
#libkspell2_la_LIBADD = libkspell2_noinst.la ../libkspell2base.la
