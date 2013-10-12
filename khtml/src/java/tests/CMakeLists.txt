set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KHTML_SOURCE_DIR}/java ${KHTML_SOURCE_DIR} )

include(ECMMarkAsTest)

########### next target ###############

set(testserver_SRCS testkjavaappletserver.cpp
    ../kjavaappletcontext.cpp  ../kjavaappletviewer.cpp  ../kjavaprocess.cpp
    ../kjavaapplet.cpp         ../kjavaappletwidget.cpp  ../kjavadownloader.cpp
    ../kjavaappletserver.cpp)


add_executable(testserver ${testserver_SRCS})
ecm_mark_as_test(testserver)

target_link_libraries(testserver
    KParts
    KDE4Support
    KF5::KI18n
    XmlGui
    KF5::KIconThemes
    KWindowSystem
    Qt5::Network
)



