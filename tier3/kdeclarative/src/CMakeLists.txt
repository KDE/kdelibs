set(KDeclarative_LIB_HEADERS
    kdeclarative.h
    qmlobject.h
    configpropertymap.h
)

foreach(_KDECLARATIVE_HEADER ${KDeclarative_LIB_HEADERS})
  set(HEADER_NAME ${KDeclarative_BINARY_DIR}/kdeclarative/${_KDECLARATIVE_HEADER})
  if (NOT EXISTS ${HEADER_NAME})
    file(WRITE ${HEADER_NAME} "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${_KDECLARATIVE_HEADER}\"\n")
  endif()
endforeach()

set(kdeclarative_SRCS
  configpropertymap.cpp
  qmlobject.cpp
  kdeclarative.cpp
  private/rootcontext.cpp
  private/kiconprovider.cpp
  private/kioaccessmanagerfactory.cpp
  private/qmlobject_p.h
)

add_library(KF5Declarative ${kdeclarative_SRCS})
generate_export_header(KF5Declarative EXPORT_FILE_NAME ${KDeclarative_BINARY_DIR}/kdeclarative/kdeclarative_export.h BASE_NAME KDeclarative)
add_library(KF5::Declarative ALIAS KF5Declarative)

target_link_libraries(KF5Declarative
  PUBLIC
    Qt5::Qml
  PRIVATE
    Qt5::Quick  #QQuickImageProvider
    KF5::ConfigCore # KCoreConfigSkeleton, in ConfigPropertyMap
    KF5::I18n       #i18n bindings
    KF5::KIOWidgets     #KIO::AccessManager
    KF5::KIconThemes #KIconProvider
)
target_include_directories(KF5Declarative PUBLIC "$<BUILD_INTERFACE:${KDeclarative_BINARY_DIR}>")

set_target_properties(KF5Declarative
  PROPERTIES VERSION ${KDECLARATIVE_VERSION_STRING}
  SOVERSION ${KDECLARATIVE_SOVERSION}
  EXPORT_NAME Declarative
)

# Install files
install(TARGETS KF5Declarative
  EXPORT KF5DeclarativeTargets
  ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
  ${KDeclarative_BINARY_DIR}/kdeclarative/kdeclarative_export.h
  ${KDeclarative_LIB_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kdeclarative
  COMPONENT Devel
)
