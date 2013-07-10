include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/ ${KDE4_INCLUDES})
include_directories( ../../src/Weaver/ ${CMAKE_CURRENT_BINARY_DIR}/../../src/Weaver )

ADD_DEFINITIONS(
    -std=c++11
)

########### next target ###############

set(HelloWorld_SRCS HelloWorld.cpp)

add_executable(ThreadWeaverHelloWorld ${HelloWorld_SRCS})

target_link_libraries(ThreadWeaverHelloWorld ${KDE4_KDECORE_LIBS} ThreadWeaver)

install(TARGETS ThreadWeaverHelloWorld DESTINATION ${BIN_INSTALL_DIR} )

########### next target ###############

set(HelloWorldRaw_SRCS HelloWorldRaw.cpp)

add_executable(ThreadWeaverHelloWorldRaw ${HelloWorld_SRCS})

target_link_libraries(ThreadWeaverHelloWorldRaw ${KDE4_KDECORE_LIBS} ThreadWeaver)

install(TARGETS ThreadWeaverHelloWorldRaw DESTINATION ${BIN_INSTALL_DIR} )
