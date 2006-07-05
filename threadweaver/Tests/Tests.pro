TEMPLATE = subdirs
SUBDIRS = JobTests.pro QueueTests.pro
OPTIONS += ordered

test.target=test
test.commands= make -f Makefile.JobTests test && make -f Makefile.QueueTests test
QMAKE_EXTRA_TARGETS += test

