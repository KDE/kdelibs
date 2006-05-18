

if(KDE4_BUILD_TESTS)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_SOURCE_DIR} )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(solidtest_SRCS 
   solidtest.cpp 
   fakedevice.cpp 
   fakemanager.cpp 
   fakeprocessor.cpp )

kde4_automoc(${solidtest_SRCS})

kde4_add_executable(solidtest RUN_UNINSTALLED ${solidtest_SRCS})

target_link_libraries(solidtest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} solid solidifaces)

add_test(solidtest ${EXECUTABLE_OUTPUT_PATH}/solidtest)

endif(KDE4_BUILD_TESTS)

########### install files ###############



