kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


set(seekslider_SRCS seekslider.cpp )
kde4_automoc(${seekslider_SRCS})
if(KDE4_BUILD_TESTS)
kde4_add_executable(seekslider ${seekslider_SRCS})
target_link_libraries(seekslider  ${QT_AND_KDECORE_LIBS} phononui ${QT_QTTEST_LIBRARY} )
endif(KDE4_BUILD_TESTS)

set(guitest_SRCS main.cpp testwidget.cpp )
kde4_automoc(${guitest_SRCS})
if(KDE4_BUILD_TESTS)
kde4_add_executable(guitest ${guitest_SRCS})
target_link_libraries(guitest  ${QT_AND_KDECORE_LIBS} phononui ${QT_QTTEST_LIBRARY} )
endif(KDE4_BUILD_TESTS)

set(mediaplayer_SRCS mediaplayer.cpp )
kde4_automoc(${mediaplayer_SRCS})
if(KDE4_BUILD_TESTS)
kde4_add_executable(mediaplayer ${mediaplayer_SRCS})
target_link_libraries(mediaplayer  ${QT_AND_KDECORE_LIBS} phononui ${QT_QTTEST_LIBRARY} )
endif(KDE4_BUILD_TESTS)


kde4_footer()
#LDADD = ../libphononui.la -lQtTest_debug
#AM_LDFLAGS = $(KDE_RPATH) $(all_libraries)
