set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

find_package(Qt5Test REQUIRED)

include(ECMAddTests)

########### next target ###############

ecm_add_test(kcookiejartest.cpp
    NAME_PREFIX "kioslave-"
    LINK_LIBRARIES  Qt5::Test KF5::KIOCore KF5::KCoreAddons
)

########### install files ###############





