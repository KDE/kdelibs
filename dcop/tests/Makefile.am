
nothing:
	@echo -e "dcop tests not run.\nRun make tests in the tests directory to make them"

INCLUDES = $(all_includes)
AM_LDFLAGS   = $(all_libraries)

METASOURCES = AUTO

####### Files

check_PROGRAMS = dcop_test driver

dcop_test_SOURCES = test.cpp test.skel
dcop_test_LDADD = $(LIB_KDECORE)
dcop_test_LDFLAGS = $(KDE_RPATH)

driver_SOURCES = driver.cpp test.stub
driver_LDADD = $(LIB_KDECORE)
driver_LDFLAGS = $(KDE_RPATH)

CLEANFILES = batch.generated definitions.generated driver.generated shell.generated test.h

BUILT_SOURCES = test.h

test-clean:
	rm batch.returns driver.returns shell.returns batch.stdout driver.stdout shell.stdout

test.h: shell.generated

shell.generated: $(srcdir)/testcases $(srcdir)/generate.pl
	perl $(srcdir)/generate.pl < $(srcdir)/testcases

check-local:
	sh $(srcdir)/run-tests.sh
