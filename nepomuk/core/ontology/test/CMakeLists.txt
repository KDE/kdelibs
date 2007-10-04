project(konto_test)

include_directories(
  ${nepomukcore_SOURCE_DIR}/ontology
)

include_directories( ${KDE4_KIO_INCLUDES} ${CMAKE_SOURCE_DIR}/konto )
include_directories( ${QT_INCLUDES} ${SOPRANO_INCLUDE_DIR} )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# tests
kde4_add_unit_test(kontotest kontotest.cpp)
target_link_libraries(kontotest nepomuk ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

kde4_add_unit_test(desktopontologyloadertest desktopontologyloadertest.cpp)
target_link_libraries(desktopontologyloadertest 
  nepomuk
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBRARY}
  ${KDE4_KIO_LIBRARY}
)
