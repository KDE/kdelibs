SUBDIRS = impl

include $(top_srcdir)/kdom/Makefile.global

INCLUDES = -I$(top_srcdir) \
	-I$(top_srcdir)/kdom \
	-I$(top_srcdir)/kdom/core \
	-I$(top_srcdir)/kdom/xpath \
	$(all_includes)

noinst_LTLIBRARIES = libkdomxpath.la
libkdomxpath_la_SOURCES = XPathEvaluatorImpl.cpp XPathExceptionImpl.cpp \
	XPathExpressionImpl.cpp XPathNSResolverImpl.cpp \
	XPathNamespaceImpl.cpp XPathResultImpl.cpp
libkdomxpath_la_LIBADD = impl/libkdomxpathimpl.la $(all_libraries)

myincludedir = $(includedir)/kdom/xpath/impl
myinclude_HEADERS = XPathEvaluatorImpl.h XPathExceptionImpl.h \
	XPathExpressionImpl.h XPathNSResolverImpl.h XPathNamespaceImpl.h \
	XPathResultImpl.h

