
INCLUDES= $(all_includes)

####### 
# you can add here more. This one gets installed 
lib_LTLIBRARIES = libkformula.la
KFORMULAVERSION=	0.0.1
KFORMULAPATCH=	0

libkformula_la_SOURCES	= box.cc kformula.cc kformulaedit.cc matrixbox.cc \
			MatrixDialog.cc kformulatoolbar.cc

include_HEADERS = box.h kformula.h kformulaedit.h matrixbox.h MatrixDialog.h kformulatoolbar.h

libkformula_la_LDFLAGS = -version-info 3:0 $(X_LDFLAGS) $(QT_LDFLAGS) $(USER_LDFLAGS)
libkformula_la_LIBADD  = $(LIB_QT) ../kdecore/libkdecore.la ../kdeui/libkdeui.la -lm

libkformula_la_METASOURCES = kformulaedit.moc kformulatoolbar.moc MatrixDialog.moc




