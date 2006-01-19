AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -I$(top_srcdir)/kspell2 -I$(top_srcdir)/kspell2/ui $(all_includes)
METASOURCES = AUTO


check_PROGRAMS = test test_filter backgroundtest \
	test_dialog test_highlighter test_configdialog test_emailquotinghighlighter

test_SOURCES = test.cpp
test_LDADD = ../ui/libkspell2.la $(LIB_KDECORE)

test_filter_SOURCES = test_filter.cpp
test_filter_LDADD = ../ui/libkspell2.la $(LIB_KDECORE)

backgroundtest_SOURCES = backgroundtest.cpp
backgroundtest_LDADD = ../ui/libkspell2.la $(LIB_KDECORE)

test_dialog_SOURCES = test_dialog.cpp
test_dialog_LDADD = ../ui/libkspell2.la $(LIB_KDECORE)

test_highlighter_SOURCES = test_highlighter.cpp
test_highlighter_LDADD = ../ui/libkspell2_noinst.la ../libkspell2base.la $(LIB_KDECORE)

test_emailquotinghighlighter_SOURCES = test_emailquotinghighlighter.cpp
test_emailquotinghighlighter_LDADD = ../ui/libkspell2_noinst.la ../libkspell2base.la $(LIB_KDECORE)

test_config_SOURCES = test_config.cpp
test_config_LDADD = ../ui/libkspell2_noinst.la ../libkspell2base.la $(LIB_KDECORE)

test_configdialog_SOURCES = test_configdialog.cpp
test_configdialog_LDADD = ../ui/libkspell2_noinst.la ../libkspell2base.la $(LIB_KDECORE)
