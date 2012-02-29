set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../src
                    ${CMAKE_CURRENT_BINARY_DIR}/../src)

set(libkauth_tests_static_SRCS
    ../src/kauthaction.cpp
    ../src/kauthactionreply.cpp
    ../src/kauthexecutejob.cpp
    ../src/AuthBackend.cpp
    # Use our "special" backends manager
    BackendsManager.cpp
    ../src/HelperProxy.cpp
    ../src/kauthhelpersupport.cpp
    TestBackend.cpp
    TestHelperProxy.cpp
)

add_library(kauth_tests_static STATIC ${libkauth_tests_static_SRCS})

target_link_libraries(kauth_tests_static ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})
