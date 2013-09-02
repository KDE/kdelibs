project(kunitconversion)

add_definitions(-DTRANSLATION_DOMAIN="libkunitconversion")

set(kunitconversion_LIB_SRCS
    kcurrencycode.cpp
    converter.cpp
    value.cpp
    unit.cpp
    unitcategory.cpp
    length.cpp
    area.cpp
    volume.cpp
    temperature.cpp
    velocity.cpp
    mass.cpp
    pressure.cpp
    energy.cpp
    currency.cpp
    power.cpp
    timeunit.cpp
    fuel_efficiency.cpp
    density.cpp
    acceleration.cpp
    force.cpp
    angle.cpp
    frequency.cpp
)

set(kunitconversion_HEADERS
    kcurrencycode.h
    converter.h
    value.h
    unit.h
    unitcategory.h
    ${CMAKE_CURRENT_BINARY_DIR}/kunitconversion_export.h
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=930)

add_library(KUnitConversion ${kunitconversion_LIB_SRCS})
generate_export_header(KUnitConversion)
target_link_libraries(KUnitConversion LINK_PUBLIC  Qt5::Core
                                      LINK_PRIVATE Qt5::Xml Qt5::Network
                                                   KI18n #i18n(), ...
                                                   KConfigCore #only KCurrencyCodePrivate::loadCurrency
)

target_include_directories(KUnitConversion PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>" "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>")

set_target_properties(KUnitConversion PROPERTIES VERSION   ${KUNITCONVERSION_VERSION_STRING}
                                                 SOVERSION ${KUNITCONVERSION_SOVERSION})

install(TARGETS KUnitConversion EXPORT KUnitConversionTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${kunitconversion_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/kunitconversion COMPONENT Devel)
