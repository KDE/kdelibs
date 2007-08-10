ALSA_VERSION_STRING(ALSA_VERSION_STR)
IF(ALSA_VERSION_STR AND NOT WIN32)
   IF(ALSA_VERSION_STR MATCHES "^1\\.(0\\.(1[4-9]|[2-9][0-9]+)|[1-9][0-9]*\\.)")
      MESSAGE(STATUS "libasound version '${ALSA_VERSION_STR}' is recent enough")
      ADD_DEFINITIONS(-DHAS_LIBASOUND_DEVICE_NAME_HINT)
   ELSE(ALSA_VERSION_STR MATCHES "^1\\.(0\\.(1[4-9]|[2-9][0-9]+)|[1-9][0-9]*\\.)")
      MESSAGE(STATUS "WARNING: libasound version '${ALSA_VERSION_STR}' is not recent enough. The recommended version is '1.0.14a'. Older versions will compile correctly, but break on dmix usage.")
   ENDIF(ALSA_VERSION_STR MATCHES "^1\\.(0\\.(1[4-9]|[2-9][0-9]+)|[1-9][0-9]*\\.)")
ENDIF(ALSA_VERSION_STR AND NOT WIN32)

IF(NOT ALSA_FOUND)
   set(ALSA_INCLUDES "")
   set(ASOUND_LIBRARY "")
ENDIF(NOT ALSA_FOUND)

include_directories(${CMAKE_SOURCE_DIR}/solid ${CMAKE_BINARY_DIR}/solid ${KDE4_KIO_INCLUDES} ${ALSA_INCLUDES})

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
