add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

set(itemmodels_SRCS
  kbreadcrumbselectionmodel.cpp
  kcheckableproxymodel.cpp
  kdescendantsproxymodel.cpp
  klinkitemselectionmodel.cpp
  kmodelindexproxymapper.cpp
  krecursivefilterproxymodel.cpp
  kselectionproxymodel.cpp
)

add_library(ItemModels ${itemmodels_SRCS})
generate_export_header(ItemModels)

target_link_libraries(ItemModels LINK_PUBLIC Qt5::Core)

set_target_properties(ItemModels PROPERTIES VERSION   ${ITEMMODELS_VERSION_STRING}
                                            SOVERSION ${ITEMMODELS_SOVERSION}
)

install(TARGETS ItemModels EXPORT ItemModelsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kbreadcrumbselectionmodel.h
  kcheckableproxymodel.h
  klinkitemselectionmodel.h
  krecursivefilterproxymodel.h
  kdescendantsproxymodel.h
  kmodelindexproxymapper.h
  kselectionproxymodel.h
  ${CMAKE_CURRENT_BINARY_DIR}/itemmodels_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
