TEMPLATE	= subdirs 
SUBDIRS		= Weaver Action Experimental Tests Examples
CONFIG		+= ordered

test.target=test
test.commands=cd Tests && make test
test.depends = all $(TARGET)
QMAKE_EXTRA_TARGETS += test


