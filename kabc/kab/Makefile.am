INCLUDES= $(all_includes) -I$(srcdir)/.. -I..
KDE_CXXFLAGS = -UQT3_SUPPORT

bin_PROGRAMS = kab2kabc

kab2kabc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kab2kabc_LDADD = ../libkabc.la
kab2kabc_SOURCES = kab2kabc.cpp \
	kabapi.cc \
	kabaddressbook.cc \
	qconfigDB.cc 

METASOURCES = AUTO
