AM_CPPFLAGS = -I$(top_srcdir)/kdecore -I$(top_srcdir) $(all_includes)

bin_PROGRAMS = kconfig_compiler

kconfig_compiler_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kconfig_compiler_LDADD = $(LIB_KDECORE)
kconfig_compiler_SOURCES = kconfig_compiler.cpp

check_PROGRAMS = example autoexample

example_LDFLAGS = $(all_libraries) $(KDE_RPATH)
example_LDADD = $(LIB_KDECORE)
example_SOURCES = example.cpp exampleprefs_base.cpp

autoexample_LDFLAGS = $(all_libraries) $(KDE_RPATH)
autoexample_LDADD = $(LIB_KDECORE)
autoexample_SOURCES = general_base.ui myoptions_base.ui autoexample.cpp

example.o exampleprefs_base.o: exampleprefs_base.h
# avoid running the below command in parallel
exampleprefs_base.cpp: exampleprefs_base.h
exampleprefs_base.cpp exampleprefs_base.h: $(srcdir)/example.kcfg kconfig_compiler $(srcdir)/exampleprefs_base.kcfg-codegen
	./kconfig_compiler $(srcdir)/example.kcfg $(srcdir)/exampleprefs_base.kcfg-codegen

METASOURCES = AUTO

CLEANFILES = exampleprefs_base.h exampleprefs_base.cpp

