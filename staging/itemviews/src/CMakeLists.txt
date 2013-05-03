add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

set(itemviews_SRCS
  kextendableitemdelegate.cpp
)

add_library(ItemViews ${itemviews_SRCS})
generate_export_header(ItemViews)

target_link_libraries(ItemViews LINK_PUBLIC Qt5::Widgets)

set_target_properties(ItemViews PROPERTIES VERSION   ${ITEMVIEWS_VERSION_STRING}
                                           SOVERSION ${ITEMVIEWS_SOVERSION}
)

install(TARGETS ItemViews EXPORT ItemViewsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kextendableitemdelegate.h
  ${CMAKE_CURRENT_BINARY_DIR}/itemviews_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
