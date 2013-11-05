set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KHTML_SOURCE_DIR}/java ${KHTML_SOURCE_DIR} )

include(ECMMarkAsTest)

########### next target ###############

set(testserver_SRCS testkjavaappletserver.cpp
    ${KHTML_SOURCE_DIR}/java/kjavaappletcontext.cpp
    ${KHTML_SOURCE_DIR}/java/kjavaappletviewer.cpp
    ${KHTML_SOURCE_DIR}/java/kjavaprocess.cpp
    ${KHTML_SOURCE_DIR}/java/kjavaapplet.cpp
    ${KHTML_SOURCE_DIR}/java/kjavaappletwidget.cpp
    ${KHTML_SOURCE_DIR}/java/kjavadownloader.cpp
    ${KHTML_SOURCE_DIR}/java/kjavaappletserver.cpp)

add_executable(testserver ${testserver_SRCS})
ecm_mark_as_test(testserver)

target_link_libraries(testserver
    KParts
    KDE4Support
    KF5::KI18n
    XmlGui
    KF5::KIconThemes
    KF5::KWindowSystem
    Qt5::Network
)



