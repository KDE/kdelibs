find_package(Qt5Test REQUIRED)
find_package(Qt5Widgets REQUIRED)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/.." "${CMAKE_CURRENT_SOURCE_DIR}/.." )

include(ECMAddTests)

ecm_add_test(httpheadertokenizetest.cpp
   NAME_PREFIX "kioslave-"
   LINK_LIBRARIES KF5::KI18n Qt5::Test Qt5::Widgets
)

ecm_add_test(httpheaderdispositiontest.cpp
   NAME_PREFIX "kioslave-"
   LINK_LIBRARIES KF5::KI18n Qt5::Test Qt5::Widgets
)

ecm_add_test(httpauthenticationtest.cpp
   NAME_PREFIX "kioslave-"
   LINK_LIBRARIES
   Qt5::Test
   Qt5::Network
   Qt5::Widgets
   KF5::KI18n
   KF5::KIOCore
   KF5::KConfigCore
   KF5::KCoreAddons
   kntlm
)

if(GSSAPI_FOUND)
  target_link_libraries(kioslave-httpauthenticationtest ${GSSAPI_LIBS})
endif(GSSAPI_FOUND)

set(httpobjecttest_SRCS
   httpobjecttest.cpp
   ${kioslave-http_SOURCE_DIR}/http.cpp
   ${kioslave-http_SOURCE_DIR}/httpauthentication.cpp
   ../httpfilter.cpp
)

ecm_add_test(${httpobjecttest_SRCS}
   TEST_NAME "kioslave-httpobjecttest"
   LINK_LIBRARIES
   Qt5::Test
   Qt5::DBus
   Qt5::Widgets
   Qt5::Network # QLocalSocket
   Qt5::Xml # QDomElement
   KF5::KI18n
   KF5::KIOCore
   KF5::KConfigCore
   kntlm
   KF5::KArchive
   Solid)
if(GSSAPI_FOUND)
  target_link_libraries(kioslave-httpobjecttest ${GSSAPI_LIBS})
endif(GSSAPI_FOUND)

add_executable(httpfiltertest httpfiltertest.cpp ../httpfilter.cpp)
target_link_libraries(httpfiltertest Qt5::Test KF5::KArchive ${ZLIB_LIBRARIES})
add_test(kiohttp-httpfiltertest httpfiltertest)

