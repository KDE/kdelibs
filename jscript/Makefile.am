
####### General stuff

INCLUDES	= @QT_INCLUDES@ @X_INCLUDES@
lib_LTLIBRARIES = libjscript.la
YFLAGS = -d
SOMAJOR =      1
SOMINOR =      0

####### Files

libjscript_la_SOURCES =	t1.yy t2.l cpp2bison.cpp jstree.cpp jsexec.cpp builtin.cpp
libjscript_la_LDFLAGS = -version-info $(SOMINOR):$(SOMAJOR) -rpath $(libdir)
libjscript_la_LIBADD = @QT_LDFLAGS@ -lqt @X_LDFLAGS@ -lX11 

include_HEADERS = bison2cpp.h jstree.h jsexec.h jserror.h builtin.h

# DO NOT DELETE THIS LINE -- make depend depends on it.



