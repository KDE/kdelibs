
INCLUDES = $(all_includes)

noinst_PROGRAMS = ctest apitest notesoff

ctest_SOURCES = ctest.c
ctest_CPPFLAGS	= -I..
ctest_LDFLAGS	= -L..
ctest_LDADD = -lkmid

apitest_SOURCES = apitest.cc
apitest_CXXFLAGS	= -I..
apitest_LDFLAGS	= -L..
apitest_LDADD = -lkmid 

notesoff_SOURCES = notesoff.cc
notesoff_CXXFLAGS = -I..
notesoff_LDFLAGS = -L..
notesoff_LDADD = -lkmid 


DISTCLEANFILES = ./.deps/* ./.deps/.P
