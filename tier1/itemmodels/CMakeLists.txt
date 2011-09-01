project(itemmodels)

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
)

include(FeatureSummary)

add_subdirectory(src)
add_subdirectory(autotests)

install(FILES
  #${CMAKE_CURRENT_BINARY_DIR}/itemmodels_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
