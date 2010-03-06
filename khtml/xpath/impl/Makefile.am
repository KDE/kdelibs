include $(top_srcdir)/kdom/Makefile.global

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/kdom \
	   -I$(top_srcdir)/kdom/core \
	   -I$(top_srcdir)/kdom/parser \
	   -I$(top_srcdir)/kdom/xpath \
	   $(all_includes)

YACC = bison

noinst_LTLIBRARIES = libkdomxpathimpl.la

check_PROGRAMS = tokenizer_tester parser_tester interpreter_tester

libkdomxpathimpl_la_SOURCES = expression.cpp functions.cpp \
	parsedstatement.cpp path.cpp predicate.cpp step.cpp \
	parser.cpp tokenizer.cpp util.cpp variablereference.cpp
libkdomxpathimpl_la_LDFLAGS = $(all_libraries)

tokenizer_tester_SOURCES = tokenizer_tester.cpp
tokenizer_tester_LDADD = libkdomxpathimpl.la $(top_builddir)/kdom/libkdom.la
tokenizer_tester_LDFLAGS = $(all_libraries)

parser_tester_SOURCES = parser_tester.cpp
parser_tester_LDADD = libkdomxpathimpl.la $(top_builddir)/kdom/libkdom.la
parser_tester_LDFLAGS = $(all_libraries)

interpreter_tester_SOURCES = interpreter_tester.cpp
interpreter_tester_LDADD = libkdomxpathimpl.la $(top_builddir)/kdom/libkdom.la $(top_builddir)/kdom/xpath/libkdomxpath.la
interpreter_tester_LDFLAGS = $(all_libraries)

EXTRA_DIST = parser.y
CLEANFILES = $(top_srcdir)/kdom/xpath/impl/parser.cpp $(top_srcdir)/kdom/xpath/impl/parser.h $(top_srcdir)/kdom/xpath/impl/parser.output

parser.cpp: $(srcdir)/parser.y
$(top_srcdir)/kdom/xpath/impl/parser.cpp: $(srcdir)/parser.y
	cd $(srcdir); \
	$(YACC) -v -d -p xpathyy parser.y && mv parser.tab.c $(top_srcdir)/kdom/xpath/impl/parser.cpp; \
    if test -f parser.tab.h; then \
    if cmp -s parser.tab.h parser.h; then rm -f parser.tab.h; \
    else mv parser.tab.h $(top_srcdir)/kdom/xpath/impl/parser.h; fi \
    else :; fi
