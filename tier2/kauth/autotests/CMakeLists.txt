
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../src
                    ${CMAKE_CURRENT_BINARY_DIR}/../src)

qt4_add_dbus_adaptor(kauth_dbus_adaptor_tests_SRCS
                     ../src/backends/dbus/org.kde.auth.xml
                     ../src/backends/dbus/DBusHelperProxy.h
                     KAuth::DBusHelperProxy)

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
    ../src/backends/dbus/DBusHelperProxy.cpp
    ${kauth_dbus_adaptor_tests_SRCS}
)

add_library(kauth_tests_static STATIC ${libkauth_tests_static_SRCS})

target_link_libraries(kauth_tests_static ${QT_QTDBUS_LIBRARY} ${KCoreAddons_LIBRARIES})

include(ECMMarkAsTest)

########### next target ###############

set(SetupActionTest_SRCS SetupActionTest.cpp)

add_executable(KAuthSetupActionTest ${SetupActionTest_SRCS})
add_test(KAuthSetupActionTest KAuthSetupActionTest)
ecm_mark_as_test(KAuthSetupActionTest)

target_link_libraries(KAuthSetupActionTest ${QT_QTTEST_LIBRARY} kauth_tests_static)

########### next target ###############

set(HelperTest_SRCS HelperTest.cpp TestHelper.cpp)

add_executable(KAuthHelperTest ${HelperTest_SRCS})
add_test(KAuthHelperTest KAuthHelperTest)
ecm_mark_as_test(KAuthHelperTest)

target_link_libraries(KAuthHelperTest ${QT_QTTEST_LIBRARY} kauth_tests_static)
