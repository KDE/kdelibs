project(nepomuk_tests)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_definitions(-DDISABLE_NEPOMUK_LEGACY=1)

# Query tests
# --------------------------------------------
kde4_add_unit_test(queryparsertest NOGUI queryparsertest.cpp)
target_link_libraries(queryparsertest nepomuk nepomukquery
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
)

set(querytest_SRC querytest.cpp ../query/query_p.cpp)
soprano_add_ontology(querytest_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig" "NIE" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(querytest_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig" "NFO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(querytest_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig" "NCO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(querytest_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig" "PIMO" "Nepomuk::Vocabulary" "trig")
kde4_add_unit_test(querytest NOGUI ${querytest_SRC})
target_link_libraries(querytest nepomuk nepomukquery
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
)

set(queryserializertest_SRC queryserializertest.cpp)
soprano_add_ontology(queryserializertest_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig" "NIE" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(queryserializertest_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig" "NFO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(queryserializertest_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig" "NCO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(queryserializertest_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig" "PIMO" "Nepomuk::Vocabulary" "trig")
kde4_add_unit_test(queryserializertest NOGUI ${queryserializertest_SRC})
target_link_libraries(queryserializertest nepomuk nepomukquery
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
)



# Resource tests
# --------------------------------------------

# The basis of all the tests handling statements
kde4_add_library(nepomuk_testbase STATIC testbase.cpp)
target_link_libraries(nepomuk_testbase nepomuk ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

set(resourcetest_SRCS resourcetest.cpp)
soprano_add_ontology(resourcetest_SRCS
  ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig
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



# Ui tests
# --------------------------------------------
kde4_add_executable(ratingpaintertestapp TEST ratingpaintertestapp.cpp ratingpaintertestwidget.cpp)
target_link_libraries(ratingpaintertestapp
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  nepomuk
  )

kde4_add_executable(tagwidgetapp TEST tagwidgetapp.cpp tagwidgettest.cpp)
target_link_libraries(tagwidgetapp
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  nepomuk
  )
