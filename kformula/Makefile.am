INCLUDES= $(all_includes)

####### 
# you can add here more. This one gets installed 
lib_LTLIBRARIES = libkformula.la
KFORMULAVERSION=	0.0.1
KFORMULAPATCH=	0

# Which sources should be compiled for khexdit.
libkformula_la_SOURCES	= box.cc kformula.cc kformulaedit.cc

include_HEADERS = box.h kformula.h kformulaedit.h

libkformula_la_LDFLAGS = -version-info 3:0 $(X_LDFLAGS) $(QT_LDFLAGS) $(USER_LDFLAGS)
libkformula_la_LIBADD  = -lXext $(LIB_QT)

# just to make sure, automake makes them 
libkformula_la_METASOURCES = kformulaedit.moc

EXTRA_DIST = README

