AM_CPPFLAGS = -I$(top_srcdir)/kdecore $(all_includes)

check_PROGRAMS = test1 test2

test1_LDFLAGS = $(all_libraries) $(KDE_RPATH)
test1_LDADD = $(LIB_KDECORE)
test1_SOURCES = test1main.cpp test1.cpp

test2_LDFLAGS = $(all_libraries) $(KDE_RPATH)
test2_LDADD = $(LIB_KDECORE)
test2_SOURCES = test2main.cpp test2.cpp

test1main.o test1.o: test1.h
# avoid running the below command in parallel
test1.cpp: test1.h
test1.cpp test1.h: $(srcdir)/test1.kcfg ../kconfig_compiler $(srcdir)/test1.kcfgc
	../kconfig_compiler $(srcdir)/test1.kcfg $(srcdir)/test1.kcfgc

test2main.o test2.o: test2.h
# avoid running the below command in parallel
test2.cpp: test2.h
test2.cpp test2.h: $(srcdir)/test2.kcfg ../kconfig_compiler $(srcdir)/test2.kcfgc
	../kconfig_compiler $(srcdir)/test2.kcfg $(srcdir)/test2.kcfgc
