
lib_LTLIBRARIES = libkjava.la
libkjava_la_SOURCES = dummy.cpp

dummy.cpp:
	echo "#ifdef _AIX" > $@
	echo "void *not_empty_file;" >> $@
	echo "#endif" >> $@

libkjava_la_LDFLAGS = -version-info 1:0

