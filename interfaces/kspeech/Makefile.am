# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = $(all_includes)
AM_CPPFLAGS = $(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# LIBRARY SECTION
#########################################################################
# This is the library that gets installed.  It's name is used for all
# of the other Makefile.am variables.
lib_LTLIBRARIES =  libktts.la

# The source, library search path, and link libraries.
libktts_la_SOURCES =  ktts.cpp kspeech.skel kspeech.stub kspeechsink.skel
libktts_la_LDFLAGS = -avoid-version -module $(all_libraries)

# Header files that should not be installed.
noinst_HEADERS = ktts.h

# Header files to be installed.
include_HEADERS = kspeech.h kspeech_stub.h kspeechsink.h

# Generate API documentation.
include $(top_srcdir)/admin/Doxyfile.am
