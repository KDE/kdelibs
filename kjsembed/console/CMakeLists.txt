kde4_header()

project(kjsembed-console)

include_directories( ${CMAKE_SOURCE_DIR}/kjsembed ${CMAKE_SOURCE_DIR}/kjsembed/kjsembed ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(console_SRCS
   main.cpp
   console.cpp
   kjs_object_model.cpp
   numberedtextview.cpp
   )

kde4_automoc(${console_SRCS})

kde4_add_ui_files(console_SRCS jsconsole.ui )

qt4_add_resources(console_SRCS console.qrc )

kde4_add_executable(console ${console_SRCS})

target_link_libraries(console  ${QT_AND_KDECORE_LIBS} kjsembed )

install_targets(/bin console )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kjsembed $(all_includes)
#
#bin_PROGRAMS = console 
#
#console_SOURCES = jsconsole.ui console.qrc main.cpp console.cpp \
#                  kjs_object_model.cpp numberedtextview.cpp
#console_METASOURCES = AUTO
#console_LDFLAGS = $(all_libraries) 
#console_LDADD = ../kjsembed/libkjsembed.la
