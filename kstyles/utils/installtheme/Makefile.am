
# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

####### This part is very settheme specific
# you can add here more. This one gets installed 
bin_PROGRAMS = 	kinstalltheme

# Which sources should be compiled for settheme.
kinstalltheme_SOURCES = main.cpp 

# the library search path. 
kinstalltheme_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kinstalltheme_LDADD   = $(LIB_KDEUI)

