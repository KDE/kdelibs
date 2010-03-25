set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_definitions(-DDISABLE_NEPOMUK_LEGACY=1)

kde4_add_unit_test(queryparsertest NOGUI queryparsertest.cpp)
target_link_libraries(queryparsertest nepomukquery
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
)

set(querytest_SRC querytest.cpp ../query_p.cpp)
soprano_add_ontology(querytest_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig" "NIE" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(querytest_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig" "NFO" "Nepomuk::Vocabulary" "trig")
kde4_add_unit_test(querytest NOGUI ${querytest_SRC})
target_link_libraries(querytest nepomuk nepomukquery
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
)
