TEMPLATE = subdirs 
SUBDIRS = Construction Jobs SMIV QProcessJob

test.target=test
test.commands= cd .. && make test
QMAKE_EXTRA_TARGETS += test
