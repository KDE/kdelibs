
nothing:
	@echo -e "dcop tests not run.\nRun make tests in the tests directory to make them"

INCLUDES = $(all_includes)
AM_LDFLAGS   = $(all_libraries)

METASOURCES = AUTO

####### Files

noinst_PROGRAMS = dcop_test driver

dcop_test_SOURCES = test.cpp test.skel
BUILT_SOURCES = test.h
dcop_test_LDADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_DCOP)
dcop_test_LDFLAGS = $(KDE_RPATH)

driver_SOURCES = driver.cpp test.stub
driver_LDADD = $(LIB_QT) $(LIB_KDECORE)
driver_LDFLAGS = $(KDE_RPATH)

CLEANFILES = batch.generated definitions.generated driver.generated shell.generated

test-clean:
	rm batch.returns driver.returns shell.returns batch.stdout driver.stdout shell.stdout

test.h: generate

generate: testcases generate.pl
	perl ./generate.pl < testcases

doit: dcop_test driver
	sh run-tests.sh

.PHONY: generate
