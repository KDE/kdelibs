project(kmetadata_rcgen)

set(CMAKE_MODULE_PATH
  ${kmetadata_rcgen_SOURCE_DIR}/../cmake/modules
  ${kmetadata_rcgen_SOURCE_DIR}/../../backbone/cmake/modules
)

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

find_package(GNUWIN32)
find_package(Soprano REQUIRED)

include_directories(
  ${kmetadata_rcgen_SOURCE_DIR}
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
)

set(kmetadata_rcgen_SRCS
  rcgen.cpp
  resourceclass.cpp
  ontologyparser.cpp)

kde4_automoc(${kmetadata_rcgen_SRCS})
kde4_add_executable(kmetadata_rcgen ${kmetadata_rcgen_SRCS})
target_link_libraries(kmetadata_rcgen
  ${SOPRANO_LIBRARIES}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTTEST_LIBRARY}
)

install(TARGETS kmetadata_rcgen DESTINATION ${BIN_INSTALL_DIR} )
