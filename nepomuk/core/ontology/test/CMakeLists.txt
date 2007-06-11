project(konto_test)

include_directories(
  ${nepomukcore_SOURCE_DIR}/ontology
  ${nepomukmiddleware_SOURCE_DIR}
)

include_directories( ${KDE4_KIO_INCLUDES} ${CMAKE_SOURCE_DIR}/konto )
include_directories( ${QT_INCLUDES} ${SOPRANO_INCLUDE_DIR} )

# tests
kde4_automoc(kontotest.cpp)
kde4_add_executable(kontotest kontotest.cpp)
target_link_libraries(kontotest nepomuk ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
add_test(kontotest ${EXECUTABLE_OUTPUT_PATH}/kontotest)

kde4_automoc(desktopontologyloadertest.cpp)
kde4_add_executable(desktopontologyloadertest desktopontologyloadertest.cpp)
target_link_libraries(desktopontologyloadertest 
  nepomuk
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBRARY}
  ${KDE4_KIO_LIBRARY}
)
add_test(desktopontologyloadertest ${EXECUTABLE_OUTPUT_PATH}/desktopontologyloadertest)
