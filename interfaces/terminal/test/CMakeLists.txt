kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(test_SRCS
main.cc
)

kde4_automoc(${test_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(test ${test_SRCS})

target_link_libraries(test  ${QT_AND_KDECORE_LIBS} kdeui kio )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES=$(all_includes)
#check_PROGRAMS=test
#test_SOURCES=main.cc
#test_LDADD=$(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) -lkio
#test_LDFLAGS=$(all_libraries)
#METASOURCES=AUTO
