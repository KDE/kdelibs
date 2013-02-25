
include_directories(
    ${CMAKE_BINARY_DIR}/staging/kservice/src # kservice_export.h
    ${CMAKE_SOURCE_DIR}/staging/kservice/src/services # kservicetrader.h
    ${CMAKE_SOURCE_DIR}/staging/kservice/src/plugin # kpluginfactory.h
    ${CMAKE_SOURCE_DIR}/staging/kservice/src/sycoca # ksycocaentry.h
    ${CMAKE_BINARY_DIR}/tier2/kconfig/src/core # kconfigcore_export.h
    ${CMAKE_SOURCE_DIR}/tier2/kconfig/src/core # ksharedconfig.h
)

add_subdirectory(kde)
add_subdirectory(xmpp)
add_subdirectory(adium)
add_subdirectory(pidgin)
