AM_CPPFLAGS = -I$(top_srcdir)/kdecore $(all_includes) -DQT_NO_CAST_ASCII

check_PROGRAMS = test1 test2 test3 test4

CLEANFILES = test1.cpp test1.h \
             test2.cpp test2.h \
             test3.cpp test3.h \
             test4.cpp test4.h

test1_LDFLAGS = $(all_libraries) $(KDE_RPATH)
test1_LDADD = $(LIB_KDECORE)
test1_SOURCES = test1main.cpp test1.cpp

test2_LDFLAGS = $(all_libraries) $(KDE_RPATH)
test2_LDADD = $(LIB_KDECORE)
test2_SOURCES = test2main.cpp test2.cpp

test3_LDFLAGS = $(all_libraries) $(KDE_RPATH)
test3_LDADD = $(LIB_KDECORE)
test3_SOURCES = test3main.cpp test3.cpp

test4_LDFLAGS = $(all_libraries) $(KDE_RPATH)
test4_LDADD = $(LIB_KDECORE)
test4_SOURCES = test4main.cpp test4.cpp

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

test3main.o test3.o: test3.h
# avoid running the below command in parallel
test3.cpp: test3.h
test3.cpp test3.h: $(srcdir)/test3.kcfg ../kconfig_compiler $(srcdir)/test3.kcfgc
	../kconfig_compiler $(srcdir)/test3.kcfg $(srcdir)/test3.kcfgc

test4main.o test4.o: test4.h
# avoid running the below command in parallel
test4.cpp: test4.h
test4.cpp test4.h: $(srcdir)/test4.kcfg ../kconfig_compiler $(srcdir)/test4.kcfgc
	../kconfig_compiler $(srcdir)/test4.kcfg $(srcdir)/test4.kcfgc

