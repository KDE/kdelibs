INCLUDES = -I$(top_srcdir)/khtml $(all_includes)

# These are not really libraries, but modules dynamically opened.
# So they should be installed in kde_module_dir, which is usually $kde_prefix/lib/kde3
kde_module_LTLIBRARIES = libkmultipart.la

libkmultipart_la_SOURCES = kmultipart.cpp
libkmultipart_la_LIBADD = $(LIB_KPARTS)
libkmultipart_la_DEPENDENCIES = $(LIB_KPARTS)
libkmultipart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)

# Automatically generate moc files
METASOURCES = AUTO

# Install the .desktop file into the kde_services directory
kde_services_DATA = kmultipart.desktop

