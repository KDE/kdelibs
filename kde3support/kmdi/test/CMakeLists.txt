kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kde3support/kmdi ${CMAKE_SOURCE_DIR}/kde3support/kparts ${CMAKE_SOURCE_DIR}/kde3support/kdeui ${CMAKE_SOURCE_DIR}/kde3support ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfourchildren_SRCS
hello.cpp
main.cpp
mainwidget.cpp
)

kde4_automoc(${kfourchildren_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kfourchildren ${kfourchildren_SRCS})

target_link_libraries(kfourchildren  ${QT_AND_KDECORE_LIBS} kutils kde3support )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#check_PROGRAMS = kfourchildren
#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/kde3support/kmdi -I$(top_srcdir)/kde3support/kparts \
#	   -I$(top_srcdir)/kde3support/kdeui -I$(top_srcdir)/kde3support -I.. $(all_includes)
#
#kfourchildren_SOURCES = hello.cpp main.cpp mainwidget.cpp
#kfourchildren_LDFLAGS = -no-undefined $(all_libraries)
#kfourchildren_LDADD =   ../../../kutils/libkutils.la ../../libkde3support.la
#
