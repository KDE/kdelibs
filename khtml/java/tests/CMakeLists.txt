set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/khtml/java ${CMAKE_SOURCE_DIR}/khtml )


########### next target ###############

set(testserver_SRCS testkjavaappletserver.cpp
    ../kjavaappletcontext.cpp  ../kjavaappletviewer.cpp  ../kjavaprocess.cpp
    ../kjavaapplet.cpp         ../kjavaappletwidget.cpp  ../kjavadownloader.cpp
    ../kjavaappletserver.cpp)


kde4_add_executable(testserver TEST ${testserver_SRCS})

target_link_libraries(testserver  ${KDE4_KDECORE_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KDEUI_LIBS})



