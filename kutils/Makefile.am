INCLUDES = -I$(top_srcdir)/interfaces/kregexpeditor $(all_includes)

lib_LTLIBRARIES   = libkutils.la
libkutils_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 2:0:1
# We don't need KIO right now, but we will need KTrader for kregexpeditor
libkutils_la_LIBADD  = ../kio/libkio.la

libkutils_la_SOURCES = kfind.cpp kfinddialog.cpp kreplace.cpp kreplacedialog.cpp

include_HEADERS = kfind.h kfinddialog.h kreplace.h kreplacedialog.h

METASOURCES = AUTO

DOXYGEN_REFERENCES = kdecore kdeui
include ../admin/Doxyfile.am
