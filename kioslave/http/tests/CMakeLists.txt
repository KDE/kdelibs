set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/..
                    ${CMAKE_SOURCE_DIR}/kio/httpfilter)

kde4_add_unit_test(httpheadertokenizetest httpheadertokenizetest.cpp)
target_link_libraries(httpheadertokenizetest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY}
                      ${QT_QTGUI_LIBRARY})

kde4_add_unit_test(httpheaderdispositiontest httpheaderdispositiontest.cpp)
target_link_libraries(httpheaderdispositiontest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY}
                      ${QT_QTGUI_LIBRARY})

kde4_add_unit_test(httpauthenticationtest httpauthenticationtest.cpp)
target_link_libraries(httpauthenticationtest ${KDE4_KDECORE_LIBS} kntlm ${QT_QTTEST_LIBRARY}
                      ${QT_QTNETWORK_LIBRARY} ${QT_QTGUI_LIBRARY})
if(GSSAPI_FOUND)
  target_link_libraries(httpauthenticationtest ${GSSAPI_LIBS})
endif(GSSAPI_FOUND)

kde4_add_unit_test(httpobjecttest httpobjecttest.cpp
   ${kioslave-http_SOURCE_DIR}/http.cpp # requires solid
   ${kioslave-http_SOURCE_DIR}/httpauthentication.cpp
   ${httpfilter_STAT_SRCS}
   ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc

)
target_link_libraries(httpobjecttest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY}
                      ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} kntlm)
if(GSSAPI_FOUND)
  target_link_libraries(httpobjecttest ${GSSAPI_LIBS})
endif(GSSAPI_FOUND)
