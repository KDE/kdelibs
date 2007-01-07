macro_optional_find_package(Alsa)
alsa_configure_file(${CMAKE_BINARY_DIR}/config-alsa.h)

if (HAVE_LIBASOUND2)
    include_directories(${CMAKE_SOURCE_DIR}/solid)
    add_subdirectory( tests )

    set(kalsadevicelist_LIB_SRCS
        alsadevice.cpp
        alsadeviceenumerator.cpp
        )
    kde4_automoc(${kalsadevicelist_LIB_SRCS})
    kde4_add_library(kalsadevicelist SHARED ${kalsadevicelist_LIB_SRCS})
    target_link_libraries(kalsadevicelist ${KDE4_KDECORE_LIBS} solid ${ASOUND_LIBRARY})

    set_target_properties(kalsadevicelist PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

    install(TARGETS kalsadevicelist  DESTINATION ${LIB_INSTALL_DIR})
    install(FILES
        alsadevice.h
        alsadeviceenumerator.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/phonon)
endif (HAVE_LIBASOUND2)
