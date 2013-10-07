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
  bindings/i18n.cpp
  bindings/icon.cpp
  bindings/qscriptnonguibookkeeping.cpp
  bindings/url.cpp
)

add_library(KDeclarative ${kdeclarative_SRCS})
generate_export_header(KDeclarative EXPORT_FILE_NAME ${KDeclarative_BINARY_DIR}/kdeclarative/kdeclarative_export.h)

target_link_libraries(KDeclarative
  LINK_PUBLIC
    Qt5::Qml
  LINK_PRIVATE
    Qt5::Quick  #QQuickImageProvider
    Qt5::Script #QScriptValue, QScriptEngine (see bindings/*.cpp)
    KConfigCore # KCoreConfigSkeleton, in ConfigPropertyMap
    KI18n       #i18n bindings
    KIconThemes #KIconProvider
    KCoreAddons #qscriptnonguibookkeeping.cpp
    KIOCore     #qscriptnonguibookkeeping.cpp
)
target_include_directories(KDeclarative PUBLIC "$<BUILD_INTERFACE:${KDeclarative_BINARY_DIR}>")

set_target_properties(KDeclarative
  PROPERTIES VERSION ${KDECLARATIVE_VERSION_STRING}
  SOVERSION ${KDECLARATIVE_SOVERSION}
)

# Install files
install(TARGETS KDeclarative
  EXPORT KDeclarativeTargets
  ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
  ${KDeclarative_BINARY_DIR}/kdeclarative/kdeclarative_export.h
  ${KDeclarative_LIB_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kdeclarative
  COMPONENT Devel
)
