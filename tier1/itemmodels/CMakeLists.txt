project(itemmodels)

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
)

include(FeatureSummary)

add_subdirectory(src)
add_subdirectory(autotests)

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
