
####### General stuff

INCLUDES	= @QT_INCLUDES@ @X_INCLUDES@
lib_LTLIBRARIES = libjscript.la
YACC		= bison -y
LEX		= flex
YFLAGS 		= -d
SOMAJOR 	= 1
SOMINOR 	= 0

####### Files

libjscript_la_SOURCES =	t1.c t2.c cpp2bison.cpp jstree.cpp jsexec.cpp builtin.cpp
libjscript_la_LDFLAGS = -version-info $(SOMINOR):$(SOMAJOR) -rpath $(libdir)
libjscript_la_LIBADD = @QT_LDFLAGS@ -lqt @X_LDFLAGS@ -lX11 -ldl

include_HEADERS = bison2cpp.h jstree.h jsexec.h jserror.h builtin.h

t1.c: t1.y
	$(YACC) $(YFLAGS) t1.y $< && mv y.tab.c $@
	if test -f y.tab.h; then \
	if cmp -s y.tab.h $*.h; then rm -f y.tab.h; \
	else mv y.tab.h $*.h; fi \
	else :; fi

t1.h: t1.y
	$(YACC) $(YFLAGS) t1.y $< && mv y.tab.c $@
	if test -f y.tab.h; then \
	if cmp -s y.tab.h $*.h; then rm -f y.tab.h; \
	else mv y.tab.h $*.h; fi \
	else :; fi

t2.c: t2.l
	$(LEX) $< && mv lex.yy.c $@

t2.c: t1.h
