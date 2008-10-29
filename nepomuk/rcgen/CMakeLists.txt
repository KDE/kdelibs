project(nepomuk_rcgen)

include_directories(
  ${nepomuk_rcgen_SOURCE_DIR}
  ${KDE4_KDECORE_INCLUDES}
  ${QT_INCLUDES}
  ${SOPRANO_INCLUDE_DIR} )

set(nepomuk_rcgen_SRCS
  rcgen.cpp
  resourceclass.cpp
  ontologyparser.cpp)

kde4_add_executable(nepomuk-rcgen ${nepomuk_rcgen_SRCS})
target_link_libraries(nepomuk-rcgen
  ${SOPRANO_LIBRARIES}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
)

install(TARGETS nepomuk-rcgen ${INSTALL_TARGETS_DEFAULT_ARGS} )
