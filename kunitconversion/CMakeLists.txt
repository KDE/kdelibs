project(kunitconversion)

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(KUNITCONVERSION_NO_SOLID TRUE)
    set(KUNITCONVERSION_NO_KIO TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kunitconversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kunitconversion.h)

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
    ${CMAKE_CURRENT_BINARY_DIR}/kunitconversion_export.h
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=930)

if(NOT KUNITCONVERSION_NO_KIO)
    include_directories(${KDE4_KIO_INCLUDES})
    set(KUNITCONVERSION_EXTRA_LIBS ${KUNITCONVERSION_EXTRA_LIBS} ${KDE4_KIO_LIBS})
else(NOT KUNITCONVERSION_NO_KIO)
    set(KUNITCONVERSION_EXTRA_LIBS ${KUNITCONVERSION_EXTRA_LIBS} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY})
endif(NOT KUNITCONVERSION_NO_KIO)

if(NOT KUNITCONVERSION_NO_SOLID)
    include_directories(${Solid_INCLUDE_DIRS})
    set(KUNITCONVERSION_EXTRA_LIBS ${KUNITCONVERSION_EXTRA_LIBS} ${Solid_LIBRARIES})
endif(NOT KUNITCONVERSION_NO_SOLID)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_KDECORE_INCLUDES})
include_directories (${ki18n_SOURCE_DIR}/src ${ki18n_BINARY_DIR}/src)

add_subdirectory(tests)

kde4_add_library(kunitconversion ${LIBRARY_TYPE} ${kunitconversion_LIB_SRCS})
generate_export_header(kunitconversion)
target_link_libraries(kunitconversion ${KDE4_KDECORE_LIBS} ${KUNITCONVERSION_EXTRA_LIBS} kcoreaddons ki18n)
set_target_properties(kunitconversion PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kunitconversion EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${kunitconversion_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/kunitconversion COMPONENT Devel)
