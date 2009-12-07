set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

kde4_add_unit_test(queryparsertest NOGUI queryparsertest.cpp)
target_link_libraries(queryparsertest nepomukquery
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  ${NEPOMUK_LIBRARIES}
)

# we use private non-exported methods. Thus, we need to compile it all ourselves
set(querytest_SRC 
  ../term.cpp
  ../negationterm.cpp
  ../orterm.cpp
  ../andterm.cpp
  ../comparisonterm.cpp
  ../simpleterm.cpp
  ../groupterm.cpp
  ../resourceterm.cpp
  ../literalterm.cpp
  ../resourcetypeterm.cpp
  ../query.cpp
  ../filequery.cpp
  querytest.cpp)
soprano_add_ontology(querytest_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig" "NIE" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(querytest_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig" "NFO" "Nepomuk::Vocabulary" "trig")
kde4_add_unit_test(querytest NOGUI ${querytest_SRC})
target_link_libraries(querytest
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  ${NEPOMUK_LIBRARIES}
  nepomuk
)
