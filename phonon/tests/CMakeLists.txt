kde4_header()
add_subdirectory( fakebackend ) 
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

set(mediaobjecttest_SRCS mediaobjecttest.cpp )
set(audiopathtest_SRCS audiopathtest.cpp )
set(backendcapabilitiestest_SRCS backendcapabilitiestest.cpp )
set(audiooutputtest_SRCS audiooutputtest.cpp )
set(audiodataoutputtest_SRCS audiodataoutputtest.cpp )

kde4_automoc(${mediaobjecttest_SRCS} ${audiopathtest_SRCS} ${backendcapabilitiestest_SRCS} ${audiooutputtest_SRCS} ${audiodataoutputtest_SRCS})

set(phononcoretests_LIBS ${QT_AND_KDECORE_LIBS} phononcore ${QT_QTTEST_LIBRARY})

if(KDE4_BUILD_TESTS)
  kde4_add_executable(mediaobjecttest ${mediaobjecttest_SRCS})
  kde4_add_executable(audiopathtest ${audiopathtest_SRCS})
  kde4_add_executable(backendcapabilitiestest ${backendcapabilitiestest_SRCS})
  kde4_add_executable(audiooutputtest ${audiooutputtest_SRCS})
  kde4_add_executable(audiodataoutputtest ${audiodataoutputtest_SRCS})

  target_link_libraries(mediaobjecttest  ${phononcoretests_LIBS} )
  target_link_libraries(audiopathtest  ${phononcoretests_LIBS} )
  target_link_libraries(backendcapabilitiestest  ${phononcoretests_LIBS} )
  target_link_libraries(audiooutputtest  ${phononcoretests_LIBS} )
  target_link_libraries(audiodataoutputtest  ${phononcoretests_LIBS} )
endif(KDE4_BUILD_TESTS)

kde4_footer()
