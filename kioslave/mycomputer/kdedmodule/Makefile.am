kde_module_LTLIBRARIES = kded_mountwatcher.la

INCLUDES = $(all_includes)
kded_mountwatcher_la_SOURCES = mountwatcher.cpp mountwatcher.skel disklist.cpp disks.cpp
kded_mountwatcher_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_mountwatcher_la_LIBADD = $(LIB_KSYCOCA)

METASOURCES = AUTO

servicesdir = $(kde_servicesdir)/kded
services_DATA = mountwatcher.desktop

