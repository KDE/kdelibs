include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/ ${KDE4_INCLUDES})
include_directories( ../../src/Weaver/ ${CMAKE_CURRENT_BINARY_DIR}/../../src/Weaver )

ADD_DEFINITIONS(
    -std=c++11
)

set(HelloWorld_SRCS HelloWorld.cpp)

add_executable(ThreadweaverHelloWorld ${HelloWorld_SRCS})

target_link_libraries(ThreadweaverHelloWorld ${KDE4_KDECORE_LIBS} ThreadWeaver)

install(TARGETS ThreadweaverHelloWorld DESTINATION ${BIN_INSTALL_DIR} )
