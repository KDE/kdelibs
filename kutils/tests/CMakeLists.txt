kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfindtest_SRCS
kfindtest.cpp
)

kde4_automoc(${kfindtest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kfindtest ${kfindtest_SRCS})

target_link_libraries(kfindtest  ${QT_AND_KDECORE_LIBS} kutils )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kreplacetest_SRCS
kreplacetest.cpp
)

kde4_automoc(${kreplacetest_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kreplacetest ${kreplacetest_SRCS})

target_link_libraries(kreplacetest  ${QT_AND_KDECORE_LIBS} kutils )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#check_PROGRAMS = kfindtest kreplacetest
#
#TESTS = kfindtest kreplacetest
#
#kfindtest_SOURCES = kfindtest.cpp
#kfindtest_LDADD = ../libkutils.la
#kfindtest_LDFLAGS = $(all_libraries)
#
#kreplacetest_SOURCES = kreplacetest.cpp
#kreplacetest_LDADD = ../libkutils.la
#kreplacetest_LDFLAGS = $(all_libraries)
#
#METASOURCES = AUTO
#
