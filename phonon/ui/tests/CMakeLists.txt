if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(seekslider_SRCS seekslider.cpp )
kde4_automoc(${seekslider_SRCS})
kde4_add_executable(seekslider NOGUI ${seekslider_SRCS})
target_link_libraries(seekslider  ${KDE4_KDECORE_LIBS} phononui ${QT_QTTEST_LIBRARY} )

set(guitest_SRCS guitest.cpp )
kde4_automoc(${guitest_SRCS})
kde4_add_executable(guitest NOGUI ${guitest_SRCS})
target_link_libraries(guitest  ${KDE4_KDECORE_LIBS} phononui ${QT_QTTEST_LIBRARY} )

set(mediaplayer_SRCS mediaplayer.cpp )
kde4_automoc(${mediaplayer_SRCS})
kde4_add_executable(mediaplayer NOGUI ${mediaplayer_SRCS})
target_link_libraries(mediaplayer  ${KDE4_KDECORE_LIBS} phononui ${QT_QTTEST_LIBRARY} )

set(uimethodtest_SRCS methodtest.cpp )
kde4_automoc(${uimethodtest_SRCS})
  kde4_add_executable(uimethodtest NOGUI ${uimethodtest_SRCS})
  target_link_libraries(uimethodtest  ${KDE4_KDECORE_LIBS} phononui ${QT_QTTEST_LIBRARY} )
  add_test(uimethodtest ${EXECUTABLE_OUTPUT_PATH}/uimethodtest)
endif(KDE4_BUILD_TESTS)


#LDADD = ../libphononui.la -lQtTest_debug
#AM_LDFLAGS = $(KDE_RPATH) $(all_libraries)
