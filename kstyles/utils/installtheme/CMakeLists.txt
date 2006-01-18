kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kinstalltheme_SRCS
main.cpp
)

kde4_automoc(${kinstalltheme_SRCS})

kde4_add_executable(kinstalltheme ${kinstalltheme_SRCS})

target_link_libraries(kinstalltheme  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(/bin kinstalltheme )


########### install files ###############


kde4_footer()



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
