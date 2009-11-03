project(nepomuk_rcgen)

include_directories(
  ${nepomuk_rcgen_SOURCE_DIR}
  ${KDE4_INCLUDES}
  ${KDE4_KDEUI_INCLUDES}
  ${QT_INCLUDES}
  ${SOPRANO_INCLUDE_DIR} )

set(nepomuk_rcgen_SRCS
  codegenerator.cpp
  fastcode.cpp
  property.cpp
  rcgen.cpp
  resourceclass.cpp
  safecode.cpp
  ontologyparser.cpp)

qt4_add_resources(nepomuk_rcgen_SRCS templates.qrc)
kde4_add_executable(nepomuk-rcgen NOGUI ${nepomuk_rcgen_SRCS})
target_link_libraries(nepomuk-rcgen
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${SOPRANO_LIBRARIES}
  kdecore
  kdeui
)

install(TARGETS nepomuk-rcgen ${INSTALL_TARGETS_DEFAULT_ARGS} )
