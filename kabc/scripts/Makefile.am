../addressee.h ../addressee.cpp ../field.cpp: \
				$(srcdir)/makeaddressee \
				$(srcdir)/addressee.src.cpp \
				$(srcdir)/addressee.src.h \
				$(srcdir)/entrylist \
				$(srcdir)/field.src.cpp
	perl $(srcdir)/makeaddressee

CLEANFILES = ../addressee.h ../addressee.cpp ../field.cpp
all-local: $(CLEANFILES)

