kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(dcopidl2cpp_SRCS
main.cpp
skel.cpp
stub.cpp
stubimpl.cpp
)

kde4_automoc(${dcopidl2cpp_SRCS})

kde4_add_executable(dcopidl2cpp ${dcopidl2cpp_SRCS})

target_link_libraries(dcopidl2cpp  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin dcopidl2cpp )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES = $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#AM_LDFLAGS   = $(all_libraries)
#
######## Files
#
#bin_PROGRAMS = dcopidl2cpp
#
#dcopidl2cpp_SOURCES = main.cpp skel.cpp stub.cpp stubimpl.cpp
#noinst_HEADERS = main.h
#
#dcopidl2cpp_LDADD = $(LIB_QT) $(LIB_QTXML)
#dcopidl2cpp_LDFLAGS = $(KDE_RPATH)
#
## Run the command with ./dcopidl_test.h so that this is what gets into dcopidl_output.kidl
## (no full path, even with srcdir!=builddir)
#dcopidl_output.kidl: $(DCOPIDL) $(srcdir)/dcopidl_test.h
#	bd=`pwd`; cd $(srcdir) && $(DCOPIDL) ./dcopidl_test.h > $$bd/dcopidl_output.kidl
#
#check-local: dcopidl_output.kidl
#	./dcopidl2cpp dcopidl_output.kidl
