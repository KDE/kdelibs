include_directories(${CMAKE_SOURCE_DIR}/solid ${CMAKE_BINARY_DIR}/solid ${ALSA_INCLUDES} ${KDE4_KIO_INCLUDES})

FIND_PACKAGE(Alsa)
ALSA_VERSION_STRING(ALSA_VERSION_STR)
IF(ALSA_VERSION_STR AND NOT WIN32)
    SET(RECOMMENDED_MIN_VERSION "1.0.14")
    STRING(COMPARE LESS ${RECOMMENDED_MIN_VERSION} ${ALSA_VERSION_STR} VERSION_LESS)
    STRING(COMPARE EQUAL ${RECOMMENDED_MIN_VERSION} ${ALSA_VERSION_STR} VERSION_EQUAL)
    IF(VERSION_LESS OR VERSION_EQUAL)
        MESSAGE(STATUS "libasound version '${ALSA_VERSION_STR}' is recent enough")
    ELSE(VERSION_LESS OR VERSION_EQUAL)
        MESSAGE(STATUS "WARNING: libasound version '${ALSA_VERSION_STR}' is not recent enough. The recommended minimum version is '${RECOMMENDED_MIN_VERSION}'. Older versions will compile correctly, but break on dmix usage.")
    ENDIF(VERSION_LESS OR VERSION_EQUAL)
ENDIF(ALSA_VERSION_STR AND NOT WIN32)

set(kaudiodevicelist_LIB_SRCS
	audiodevice.cpp
	audiodeviceenumerator.cpp
	)
kde4_add_library(kaudiodevicelist SHARED ${kaudiodevicelist_LIB_SRCS})
target_link_libraries(kaudiodevicelist ${KDE4_SOLID_LIBS} ${KDE4_PHONON_LIBS} ${KDE4_KIO_LIBS} ${ASOUND_LIBRARY})

set_target_properties(kaudiodevicelist PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kaudiodevicelist DESTINATION ${LIB_INSTALL_DIR})
install(FILES
	kaudiodevicelist_export.h
	audiodevice.h
	audiodeviceenumerator.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/phonon)

# vim: noet
