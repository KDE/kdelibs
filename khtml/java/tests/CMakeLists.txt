set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/khtml/java ${CMAKE_SOURCE_DIR}/khtml )
include_directories( ${kinterprocesswindowing_SOURCE_DIR}/src ${kinterprocesswindowing_BINARY_DIR}/src )

include(ECMMarkAsTest)

########### next target ###############

set(testserver_SRCS testkjavaappletserver.cpp
    ../kjavaappletcontext.cpp  ../kjavaappletviewer.cpp  ../kjavaprocess.cpp
    ../kjavaapplet.cpp         ../kjavaappletwidget.cpp  ../kjavadownloader.cpp
    ../kjavaappletserver.cpp)


add_executable(testserver ${testserver_SRCS})
ecm_mark_as_test(testserver)

target_link_libraries(testserver  ${KDE4_KDECORE_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${kinterprocesswindowing_LIBRARY}
    kde4support
)



