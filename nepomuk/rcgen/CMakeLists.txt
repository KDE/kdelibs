project(tools)

find_package(Soprano REQUIRED)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
)

set(rcgen_SRCS
  rcgen.cpp
  resourceclass.cpp
  ontologyparser.cpp)

kde4_automoc(${rcgen_SRCS})
kde4_add_executable(rcgen ${rcgen_SRCS})
target_link_libraries(rcgen ${SOPRANO_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTTEST_LIBRARY})
