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
)

foreach(_KUNITCONVERSION_HEADER ${kunitconversion_HEADERS})
    set(HEADER_NAME ${KUnitConversion_BINARY_DIR}/kunitconversion/${_KUNITCONVERSION_HEADER})
    if (NOT EXISTS ${HEADER_NAME})
        file(WRITE ${HEADER_NAME} "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${_KUNITCONVERSION_HEADER}\"\n")
    endif()
endforeach()
if (NOT EXISTS ${KUnitConversion_BINARY_DIR}/kunitconversion/kunitconversion_export.h)
    file(WRITE ${KUnitConversion_BINARY_DIR}/kunitconversion/kunitconversion_export.h "#include \"${CMAKE_CURRENT_BINARY_DIR}/kunitconversion_export.h\"\n")
endif()

add_library(KUnitConversion ${kunitconversion_LIB_SRCS})
generate_export_header(KUnitConversion)
add_library(KF5::KUnitConversion ALIAS KUnitConversion)
target_include_directories(KUnitConversion PUBLIC "$<BUILD_INTERFACE:${KUnitConversion_BINARY_DIR}>" "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/kunitconversion>")
target_link_libraries(KUnitConversion PUBLIC  Qt5::Core
                                      PRIVATE Qt5::Xml Qt5::Network
                                              KF5::KI18n #i18n(), ...
                                              KF5::ConfigCore #only KCurrencyCodePrivate::loadCurrency
)

set_target_properties(KUnitConversion PROPERTIES VERSION   ${KUNITCONVERSION_VERSION_STRING}
                                                 SOVERSION ${KUNITCONVERSION_SOVERSION})

install(TARGETS KUnitConversion EXPORT KUnitConversionTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${kunitconversion_HEADERS}
                ${CMAKE_CURRENT_BINARY_DIR}/kunitconversion_export.h
                DESTINATION ${INCLUDE_INSTALL_DIR}/kunitconversion COMPONENT Devel)
