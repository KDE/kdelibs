
nothing:
	@echo -e "dcop tests not run.\nRun make tests in the tests directory to make them"

INCLUDES = $(all_includes)
AM_LDFLAGS   = $(all_libraries)

METASOURCES = AUTO

####### Files

check_PROGRAMS = dcop_test driver

dcop_test_SOURCES = test.cpp test.skel
dcop_test_LDADD = ../libDCOP.la
dcop_test_LDFLAGS = $(KDE_RPATH)

driver_SOURCES = driver.cpp test.stub
driver_LDADD = ../libDCOP.la
driver_LDFLAGS = $(KDE_RPATH)

CLEANFILES = batch.generated definitions.generated driver.generated shell.generated
test.o: batch.generated definitions.generated driver.generated shell.generated

test-clean:
	rm batch.returns driver.returns shell.returns batch.stdout driver.stdout shell.stdout

test.h: shell.generated

shell.generated: $(srcdir)/testcases $(srcdir)/generate.pl
	perl $(srcdir)/generate.pl < $(srcdir)/testcases

check-local: dcop_test driver
	sh $(srcdir)/run-tests.sh
