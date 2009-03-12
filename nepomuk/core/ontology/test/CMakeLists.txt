project(konto_test)

include_directories(
  ${nepomukcore_SOURCE_DIR}/ontology
)

include_directories( ${KDE4_KIO_INCLUDES} ${CMAKE_SOURCE_DIR}/konto )
include_directories( ${QT_INCLUDES} ${SOPRANO_INCLUDE_DIR} )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# tests
# TODO: fix, and turn into kde4_add_unit_test again
kde4_add_executable(entitytest entitytest.cpp)
target_link_libraries(entitytest nepomuk ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
                                         ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY}
					 ${SOPRANO_LIBRARIES})
