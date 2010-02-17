project(kunitconversion)

add_subdirectory(tests)

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
    acceleration.cpp
    force.cpp
    angle.cpp
    frequency.cpp
)

set(kunitconversion_HEADERS
    converter.h
    value.h
    unit.h
    unitcategory.h
    kunitconversion_export.h
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=930)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/solid/ ${CMAKE_BINARY_DIR}/solid/ ${KDE4_KDECORE_INCLUDES})


kde4_add_library(kunitconversion SHARED ${kunitconversion_LIB_SRCS})
target_link_libraries(kunitconversion ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS})
set_target_properties(kunitconversion PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kunitconversion EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${kunitconversion_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/kunitconversion COMPONENT Devel)
