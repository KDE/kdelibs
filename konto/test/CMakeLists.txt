project(konto_test)

include_directories(
  ${konto_SOURCE_DIR}
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${KNEPOMUK_INCLUDE_DIR}
  ${SOPRANO_INCLUDE_DIR}
)

# tests
kde4_automoc(kontotest.cpp)
kde4_add_executable(kontotest kontotest.cpp)
target_link_libraries(kontotest konto ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
add_test(kontotest ${EXECUTABLE_OUTPUT_PATH}/kontotest)

kde4_automoc(desktopontologyloadertest.cpp)
kde4_add_executable(desktopontologyloadertest desktopontologyloadertest.cpp)
target_link_libraries(desktopontologyloadertest 
  konto
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBRARY}
  ${KDE4_KIO_LIBRARY}
)
add_test(desktopontologyloadertest ${EXECUTABLE_OUTPUT_PATH}/desktopontologyloadertest)
