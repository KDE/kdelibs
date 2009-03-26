set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
project(kmetadata_tests)

include_directories(
  ${kmetadata_SOURCE_DIR}
  ${libkmetadata_SOURCE_DIR}
  ${libkmetadata_SOURCE_DIR}/generated
  ${kmetadata_ui_SOURCE_DIR}
)

include_directories( ${KDE4_KDEUI_INCLUDES} )
include_directories( ${QT_INCLUDES} ${SOPRANO_INCLUDE_DIR} )

# The basis of all the tests handling statements
kde4_add_library(nepomuk_testbase STATIC testbase.cpp)
target_link_libraries(nepomuk_testbase nepomuk ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

# tests
#TODO: fix, and turn into kde4_add_unit_test again
kde4_add_executable(resourcetest resourcetest.cpp)
target_link_libraries(resourcetest
  nepomuk_testbase
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS})

#kde4_add_unit_test(varianttest varianttest.cpp)
#target_link_libraries(varianttest kmetadata ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTTEST_LIBRARY})

kde4_add_unit_test(speedtest speedtest.cpp)
target_link_libraries(speedtest
  nepomuk
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS})

#set(datatest_SRC 
#  resourcedatatest.cpp
#  ../kmetadata/resourcedata.cpp
#)
#kde4_add_unit_test(resourcedatatest ${datatest_SRC})
#target_link_libraries(resourcedatatest
#  kmetadata_testbase
#  kmetadata
#  knepomuk
#  konto
#  ${QT_QTCORE_LIBRARY}
#  ${QT_QTDBUS_LIBRARY}
#  ${QT_QTTEST_LIBRARY}
#  ${QT_QTGUI_LIBRARY}
#  ${KDE4_KDECORE_LIBRARY})

# Not a real test, just a simple app for testing a widget manually
kde4_add_executable(tagcloudtest TEST tagcloudtest.cpp)
target_link_libraries(tagcloudtest nepomuk
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS})
