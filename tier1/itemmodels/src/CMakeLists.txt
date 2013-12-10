
set(itemmodels_SRCS
  kbreadcrumbselectionmodel.cpp
  kcheckableproxymodel.cpp
  kdescendantsproxymodel.cpp
  klinkitemselectionmodel.cpp
  kmodelindexproxymapper.cpp
  krecursivefilterproxymodel.cpp
  kselectionproxymodel.cpp
)

add_library(KF5ItemModels ${itemmodels_SRCS})
generate_export_header(KF5ItemModels BASE_NAME ItemModels)
add_library(KF5::ItemModels ALIAS KF5ItemModels)

target_link_libraries(KF5ItemModels PUBLIC Qt5::Core)

set_target_properties(KF5ItemModels PROPERTIES VERSION   ${ITEMMODELS_VERSION_STRING}
                                               SOVERSION ${ITEMMODELS_SOVERSION}
                                               EXPORT_NAME ItemModels
)

install(TARGETS KF5ItemModels EXPORT KF5ItemModelsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

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
