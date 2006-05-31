
set(seekslider_SRCS seekslider.cpp )
kde4_automoc(${seekslider_SRCS})
if(KDE4_BUILD_TESTS)
kde4_add_executable(seekslider ${seekslider_SRCS})
target_link_libraries(seekslider  ${KDE4_KDECORE_LIBS} phononui ${QT_QTTEST_LIBRARY} )
endif(KDE4_BUILD_TESTS)

set(guitest_SRCS main.cpp testwidget.cpp )
kde4_automoc(${guitest_SRCS})
if(KDE4_BUILD_TESTS)
kde4_add_executable(guitest ${guitest_SRCS})
target_link_libraries(guitest  ${KDE4_KDECORE_LIBS} phononui ${QT_QTTEST_LIBRARY} )
endif(KDE4_BUILD_TESTS)

set(mediaplayer_SRCS mediaplayer.cpp )
kde4_automoc(${mediaplayer_SRCS})
if(KDE4_BUILD_TESTS)
kde4_add_executable(mediaplayer ${mediaplayer_SRCS})
target_link_libraries(mediaplayer  ${KDE4_KDECORE_LIBS} phononui ${QT_QTTEST_LIBRARY} )
endif(KDE4_BUILD_TESTS)

set(uimethodtest_SRCS methodtest.cpp )
kde4_automoc(${uimethodtest_SRCS})
if(KDE4_BUILD_TESTS)
kde4_add_executable(uimethodtest ${uimethodtest_SRCS})
target_link_libraries(uimethodtest  ${KDE4_KDECORE_LIBS} phononui ${QT_QTTEST_LIBRARY} )
endif(KDE4_BUILD_TESTS)

add_test(uimethodtest ${EXECUTABLE_OUTPUT_PATH}/uimethodtest)

#LDADD = ../libphononui.la -lQtTest_debug
#AM_LDFLAGS = $(KDE_RPATH) $(all_libraries)
