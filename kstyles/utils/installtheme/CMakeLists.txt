


########### next target ###############

set(kinstalltheme_SRCS main.cpp )

kde4_automoc(${kinstalltheme_SRCS})

kde4_add_executable(kinstalltheme NOGUI ${kinstalltheme_SRCS})

target_link_libraries(kinstalltheme  ${KDE4_KDECORE_LIBS} kdeui )

install(TARGETS kinstalltheme DESTINATION bin)


########### install files ###############





#original Makefile.am contents follow:

#
## set the include path for X, qt and KDE
#INCLUDES= $(all_includes)
#
######## This part is very settheme specific
## you can add here more. This one gets installed 
#bin_PROGRAMS = 	kinstalltheme
#
## Which sources should be compiled for settheme.
#kinstalltheme_SOURCES = main.cpp 
#
## the library search path. 
#kinstalltheme_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
## the libraries to link against. Be aware of the order. First the libraries,
## that depend on the following ones.
#kinstalltheme_LDADD   = $(LIB_KDEUI)
#
