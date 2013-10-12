set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt5Test REQUIRED)
find_package(Qt5Widgets REQUIRED)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/.." "${CMAKE_CURRENT_SOURCE_DIR}/.." )

kde4_add_unit_test(httpheadertokenizetest httpheadertokenizetest.cpp)
target_link_libraries(httpheadertokenizetest KF5::KI18n Qt5::Test
                      Qt5::Widgets)

kde4_add_unit_test(httpheaderdispositiontest httpheaderdispositiontest.cpp)
target_link_libraries(httpheaderdispositiontest KF5::KI18n Qt5::Test
                      Qt5::Widgets)

kde4_add_unit_test(httpauthenticationtest httpauthenticationtest.cpp)
target_link_libraries(httpauthenticationtest KF5::KI18n kntlm Qt5::Test
                      Qt5::Network Qt5::Widgets KF5::KCoreAddons kio)
if(GSSAPI_FOUND)
  target_link_libraries(httpauthenticationtest ${GSSAPI_LIBS})
endif(GSSAPI_FOUND)

kde4_add_unit_test(httpobjecttest httpobjecttest.cpp
   ${kioslave-http_SOURCE_DIR}/http.cpp
   ${kioslave-http_SOURCE_DIR}/httpauthentication.cpp
   ${httpfilter_STAT_SRCS}
   ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc

)
target_link_libraries(httpobjecttest Qt5::Test Qt5::DBus
   Qt5::Widgets ${KDE4_KIO_LIBS} KF5::KI18n kntlm KF5::KArchive Solid)
if(GSSAPI_FOUND)
  target_link_libraries(httpobjecttest ${GSSAPI_LIBS})
endif(GSSAPI_FOUND)
