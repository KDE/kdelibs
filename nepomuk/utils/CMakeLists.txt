project(nepomuk_utils)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DDISABLE_NEPOMUK_LEGACY=1)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/..
)

set(nepomukutils_SRCS
  utils.cpp

  facetdelegate.cpp
  facetwidget.cpp
  facetmodel.cpp
  facetfiltermodel.cpp
  facet.cpp
  simplefacet.cpp
  dynamicresourcefacet.cpp
  datefacet.cpp
  proxyfacet.cpp

  daterange.cpp
  daterangeselectionwidget.cpp

  resourcemodel.cpp
  simpleresourcemodel.cpp
  searchlineedit.cpp
  searchwidget.cpp
)

kde4_add_ui_files(nepomukutils_SRCS daterangeselectionwidget.ui)

kde4_add_library(nepomukutils ${LIBRARY_TYPE} ${nepomukutils_SRCS})

target_link_libraries(nepomukutils
  ${SOPRANO_LIBRARIES}
  ${QT_QTCORE_LIBRARY}
  nepomuk
  nepomukquery
  kdecore
)

set_target_properties(nepomukutils PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS nepomukutils EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  utils.h
  facet.h
  simplefacet.h
  dynamicresourcefacet.h
  proxyfacet.h
  facetwidget.h
  searchwidget.h
  resourcemodel.h
  simpleresourcemodel.h
  nepomukutils_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk COMPONENT Devel
)
