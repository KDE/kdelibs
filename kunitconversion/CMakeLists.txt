project(kunitconversion)

set(kunitconversion_LIB_SRCS
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
)

set(kunitconversion_HEADERS
    converter.h
    value.h
    unit.h
    unitcategory.h
    kunitconversion_export.h
)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

kde4_add_library(kunitconversion SHARED ${kunitconversion_LIB_SRCS})
target_link_libraries(kunitconversion ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS})
set_target_properties(kunitconversion PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kunitconversion ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES FindKUnitConversion.cmake DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)
install(FILES ${kunitconversion_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/kunitconversion COMPONENT Devel)
