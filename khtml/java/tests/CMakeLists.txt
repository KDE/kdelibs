set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/khtml/java ${CMAKE_SOURCE_DIR}/khtml )

include(ECMMarkAsTest)

########### next target ###############

set(testserver_SRCS testkjavaappletserver.cpp
    ../kjavaappletcontext.cpp  ../kjavaappletviewer.cpp  ../kjavaprocess.cpp
    ../kjavaapplet.cpp         ../kjavaappletwidget.cpp  ../kjavadownloader.cpp
    ../kjavaappletserver.cpp)


add_executable(testserver ${testserver_SRCS})
ecm_mark_as_test(testserver)

target_link_libraries(testserver
    ${KDE4_KPARTS_LIBS}
    KInterProcessWindowing
    KDE4Support
)



