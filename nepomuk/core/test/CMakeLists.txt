set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

project(nepomuk_tests)

include_directories( 
  ${KDE4_KDEUI_INCLUDES}
  ${QT_INCLUDES} 
  ${SOPRANO_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )
include (SopranoAddOntology)

# The basis of all the tests handling statements
kde4_add_library(nepomuk_testbase STATIC testbase.cpp)
target_link_libraries(nepomuk_testbase nepomuk ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

set(resourcetest_SRCS resourcetest.cpp)
soprano_add_ontology(resourcetest_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/nie.trig
  "NIE"
  "Nepomuk::Vocabulary"
  "trig")
kde4_add_unit_test(resourcetest ${resourcetest_SRCS})
target_link_libraries(resourcetest
  nepomuk_testbase
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${SOPRANO_LIBRARIES}
  kdecore)
