project(kunitconversion)

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(KUNITCONVERSION_NO_SOLID TRUE)
    set(KUNITCONVERSION_NO_KIO TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)

configure_file(config-kunitconversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kunitconversion.h)

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

if(NOT KUNITCONVERSION_NO_KIO)
    set(KUNITCONVERSION_EXTRA_LIBS ${KUNITCONVERSION_EXTRA_LIBS} KIOCore Qt5::Xml)
else(NOT KUNITCONVERSION_NO_KIO)
    set(KUNITCONVERSION_EXTRA_LIBS ${KUNITCONVERSION_EXTRA_LIBS} Qt5::Network Qt5::Xml)
endif(NOT KUNITCONVERSION_NO_KIO)

if(NOT KUNITCONVERSION_NO_SOLID)
    set(KUNITCONVERSION_EXTRA_LIBS ${KUNITCONVERSION_EXTRA_LIBS} ${Solid_LIBRARIES})
endif(NOT KUNITCONVERSION_NO_SOLID)

add_library(KUnitConversion ${kunitconversion_LIB_SRCS})
generate_export_header(KUnitConversion)
target_link_libraries(KUnitConversion ${KUNITCONVERSION_EXTRA_LIBS} ${KCoreAddons_LIBRARIES} KConfigCore KI18n)
target_include_directories(KUnitConversion PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>" "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>")

set_target_properties(KUnitConversion PROPERTIES VERSION   ${KUNITCONVERSION_VERSION_STRING}
                                                 SOVERSION ${KUNITCONVERSION_SOVERSION})

install(TARGETS KUnitConversion EXPORT KUnitConversionTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${kunitconversion_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/kunitconversion COMPONENT Devel)
