TEMPLATE = subdirs 
SUBDIRS = Construction Jobs SMIV QProcessJob Actions

test.target=test
test.commands= cd .. && make test
QMAKE_EXTRA_TARGETS += test
