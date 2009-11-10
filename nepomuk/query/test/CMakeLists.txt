set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

kde4_add_unit_test(queryparsertest NOGUI queryparsertest.cpp)
target_link_libraries(queryparsertest nepomukquery2
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${SOPRANO_LIBRARIES}
  ${NEPOMUK_LIBRARIES}
)

set(querytest_SRC querytest.cpp)
soprano_add_ontology(querytest_SRC ${CMAKE_CURRENT_SOURCE_DIR}/pimo.trig "PIMO" "Nepomuk::Vocabulary" "trig")
kde4_add_unit_test(querytest NOGUI ${querytest_SRC})
target_link_libraries(querytest nepomukquery2
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${SOPRANO_LIBRARIES}
  ${NEPOMUK_LIBRARIES}
)
