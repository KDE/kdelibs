
set(itemmodels_SRCS
  kbreadcrumbselectionmodel.cpp
  kcheckableproxymodel.cpp
  kdescendantsproxymodel.cpp
  kidentityproxymodel.cpp
  klinkitemselectionmodel.cpp
  kmodelindexproxymapper.cpp
  krecursivefilterproxymodel.cpp
  kselectionproxymodel.cpp
  kviewstateserializer.cpp
  kviewstatemaintainerbase.cpp
)

add_library(itemmodels ${LIBRARY_TYPE} ${itemmodels_SRCS})
generate_export_header(itemmodels)

target_link_libraries(itemmodels ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(itemmodels PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                            SOVERSION ${ECM_SOVERSION}
)

install(TARGETS itemmodels ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
  kbreadcrumbselectionmodel.h
  kcheckableproxymodel.h
  klinkitemselectionmodel.h
  krecursivefilterproxymodel.h
  kdescendantsproxymodel.h
  kidentityproxymodel.h
  kviewstateserializer.h
  kviewstatemaintainerbase.h
  kmodelindexproxymapper.h
  kselectionproxymodel.h
  ${CMAKE_CURRENT_BINARY_DIR}/itemmodels_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
