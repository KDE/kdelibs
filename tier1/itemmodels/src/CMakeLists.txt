add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

set(itemmodels_SRCS
  kbreadcrumbselectionmodel.cpp
  kcheckableproxymodel.cpp
  kdescendantsproxymodel.cpp
  klinkitemselectionmodel.cpp
  kmodelindexproxymapper.cpp
  krecursivefilterproxymodel.cpp
  kselectionproxymodel.cpp
  kviewstateserializer.cpp
  kviewstatemaintainerbase.cpp
)

add_library(itemmodels ${itemmodels_SRCS})
generate_export_header(itemmodels)

target_link_libraries(itemmodels Qt5::Widgets)

set_target_properties(itemmodels PROPERTIES VERSION   ${ITEMMODELS_VERSION_STRING}
                                            SOVERSION ${ITEMMODELS_SOVERSION}
)

install(TARGETS itemmodels EXPORT itemmodelsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kbreadcrumbselectionmodel.h
  kcheckableproxymodel.h
  klinkitemselectionmodel.h
  krecursivefilterproxymodel.h
  kdescendantsproxymodel.h
  kviewstateserializer.h
  kviewstatemaintainerbase.h
  kmodelindexproxymapper.h
  kselectionproxymodel.h
  ${CMAKE_CURRENT_BINARY_DIR}/itemmodels_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
