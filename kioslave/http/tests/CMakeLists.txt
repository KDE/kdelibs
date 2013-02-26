set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/..
                    ${CMAKE_SOURCE_DIR}/kio/httpfilter)
include_directories( ${ki18n_SOURCE_DIR}/src )
include_directories( ${ki18n_BINARY_DIR}/src )

kde4_add_unit_test(httpheadertokenizetest httpheadertokenizetest.cpp)
target_link_libraries(httpheadertokenizetest ${KDE4_KDECORE_LIBS} ki18n ${QT_QTTEST_LIBRARY}
                      Qt5::Widgets)

kde4_add_unit_test(httpheaderdispositiontest httpheaderdispositiontest.cpp)
target_link_libraries(httpheaderdispositiontest ${KDE4_KDECORE_LIBS} ki18n ${QT_QTTEST_LIBRARY}
                      Qt5::Widgets)

kde4_add_unit_test(httpauthenticationtest httpauthenticationtest.cpp)
target_link_libraries(httpauthenticationtest ${KDE4_KDECORE_LIBS} ki18n kntlm ${QT_QTTEST_LIBRARY}
                      ${QT_QTNETWORK_LIBRARY} Qt5::Widgets kcoreaddons)
if(GSSAPI_FOUND)
  target_link_libraries(httpauthenticationtest ${GSSAPI_LIBS})
endif(GSSAPI_FOUND)

kde4_add_unit_test(httpobjecttest httpobjecttest.cpp
   ${kioslave-http_SOURCE_DIR}/http.cpp
   ${kioslave-http_SOURCE_DIR}/httpauthentication.cpp
   ${httpfilter_STAT_SRCS}
   ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc

)
target_link_libraries(httpobjecttest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY}
                      Qt5::Widgets ${KDE4_KIO_LIBS} ki18n kntlm ${KArchive_LIBRARIES})
if(GSSAPI_FOUND)
  target_link_libraries(httpobjecttest ${GSSAPI_LIBS})
endif(GSSAPI_FOUND)
