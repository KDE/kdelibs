project(nepomuk_tests)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/..
)

add_definitions(-DDISABLE_NEPOMUK_LEGACY=1)
add_definitions(-DBUILDING_NEPOMUK_TESTS)

# Query tests
# --------------------------------------------
set(queryparsertest_SRC queryparsertest.cpp)
kde4_add_unit_test(queryparsertest TESTNAME nepomuk-queryparsertest NOGUI ${queryparsertest_SRC})
target_link_libraries(queryparsertest nepomuk nepomukquery
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
)

set(querytest_SRC querytest.cpp)
kde4_add_unit_test(querytest TESTNAME nepomuk-querytest NOGUI ${querytest_SRC})
target_link_libraries(querytest nepomuk nepomukquery
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
)

set(queryserializertest_SRC queryserializertest.cpp)
kde4_add_unit_test(queryserializertest TESTNAME nepomuk-queryserializertest NOGUI ${queryserializertest_SRC})
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
kde4_add_unit_test(resourcetest TESTNAME nepomuk-resourcetest ${resourcetest_SRCS})
target_link_libraries(resourcetest
  nepomuk_testbase
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${SOPRANO_LIBRARIES}
  kdecore)



# Ui tests
# --------------------------------------------
kde4_add_executable(ratingpaintertestapp TEST ratingpaintertestapp.cpp ratingpaintertestwidget.cpp)
target_link_libraries(ratingpaintertestapp
  ${KDE4_KIO_LIBS}
  nepomuk
  )

kde4_add_executable(tagwidgetapp TEST tagwidgetapp.cpp tagwidgettest.cpp)
target_link_libraries(tagwidgetapp
  ${KDE4_KIO_LIBS}
  nepomuk
  )


# Facet tests
# -----------------------------------
kde4_add_unit_test(simplefacettest TESTNAME nepomuk-simplefacettest simplefacettest.cpp)
target_link_libraries(simplefacettest
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  nepomuk
  nepomukquery
  nepomukutils
)

kde4_add_unit_test(dynamicresourcefacettest TESTNAME nepomuk-dynamicresourcefacettest dynamicresourcefacettest.cpp)
target_link_libraries(dynamicresourcefacettest
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  nepomuk
  nepomukquery
  nepomukutils
)

kde4_add_unit_test(proxyfacettest TESTNAME nepomuk-proxyfacettest proxyfacettest.cpp)
target_link_libraries(proxyfacettest
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  nepomuk
  nepomukquery
  nepomukutils
)

kde4_add_unit_test(facetmodeltest TESTNAME nepomuk-facetmodeltest facetmodeltest.cpp)
target_link_libraries(facetmodeltest
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  nepomuk
  nepomukquery
  nepomukutils
)

kde4_add_unit_test(typefacettest TESTNAME nepomuk-typefacettest typefacettest.cpp)
target_link_libraries(typefacettest
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  nepomuk
  nepomukquery
  nepomukutils
)

