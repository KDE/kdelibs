# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

####### This part is very mktheme specific
# you can add here more. This one gets installed 
bin_PROGRAMS = 	mktheme

# Which sources should be compiled for mktheme.
mktheme_SOURCES = main.cpp 

# the library search path. 
mktheme_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
mktheme_LDADD   = $(LIB_KDEUI)


