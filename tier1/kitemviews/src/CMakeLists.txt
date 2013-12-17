
set(kitemviews_SRCS
  kcategorizedsortfilterproxymodel.cpp
  kcategorizedview.cpp
  kcategorydrawer.cpp
  kextendableitemdelegate.cpp
  kfilterproxysearchline.cpp
  klistwidgetsearchline.cpp
  ktreewidgetsearchline.cpp
  kwidgetitemdelegate.cpp
  kwidgetitemdelegatepool.cpp
)

add_library(KF5ItemViews ${kitemviews_SRCS})
generate_export_header(KF5ItemViews BASE_NAME KItemViews)
add_library(KF5::ItemViews ALIAS KF5ItemViews)


target_link_libraries(KF5ItemViews PUBLIC Qt5::Widgets)

set_target_properties(KF5ItemViews PROPERTIES VERSION   ${KITEMVIEWS_VERSION_STRING}
                                              SOVERSION ${KITEMVIEWS_SOVERSION}
                                              EXPORT_NAME ItemViews
)

install(TARGETS KF5ItemViews EXPORT KF5ItemViewsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kcategorizedsortfilterproxymodel.h
  kcategorizedview.h
  kcategorydrawer.h
  kextendableitemdelegate.h
  kfilterproxysearchline.h
  klistwidgetsearchline.h
  ktreewidgetsearchline.h
  kwidgetitemdelegate.h
  ${CMAKE_CURRENT_BINARY_DIR}/kitemviews_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
