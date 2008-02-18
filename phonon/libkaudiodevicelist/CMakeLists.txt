ALSA_VERSION_STRING(ALSA_VERSION_STR)
SET(HAVE_CURRENT_ALSA FALSE)
IF(ALSA_VERSION_STR AND NOT WIN32)
   IF(ALSA_VERSION_STR MATCHES "^1\\.(0\\.(1[4-9]|[2-9][0-9]+)|[1-9][0-9]*\\.)")
      ADD_DEFINITIONS(-DHAS_LIBASOUND_DEVICE_NAME_HINT)
      SET(HAVE_CURRENT_ALSA TRUE)
   ENDIF(ALSA_VERSION_STR MATCHES "^1\\.(0\\.(1[4-9]|[2-9][0-9]+)|[1-9][0-9]*\\.)")
ENDIF(ALSA_VERSION_STR AND NOT WIN32)
IF(NOT WIN32)
   macro_log_feature(HAVE_CURRENT_ALSA "ALSA" "current alsa-lib is needed for dmix and virtual device listing" "http://www.alsa-project.org/" FALSE "1.0.14a")
ENDIF(NOT WIN32)

IF(NOT ALSA_FOUND)
   set(ALSA_INCLUDES "")
   set(ASOUND_LIBRARY "")
ENDIF(NOT ALSA_FOUND)

include_directories(${CMAKE_SOURCE_DIR}/solid ${CMAKE_BINARY_DIR}/solid ${ALSA_INCLUDES})

set(kaudiodevicelist_LIB_SRCS
	audiodevice.cpp
	audiodeviceenumerator.cpp
   hardwaredatabase.cpp
	)
IF(ALSA_FOUND)
   QT4_ADD_RESOURCES(kaudiodevicelist_LIB_SRCS resources.qrc)
ENDIF(ALSA_FOUND)
kde4_add_library(kaudiodevicelist SHARED ${kaudiodevicelist_LIB_SRCS})
target_link_libraries(kaudiodevicelist ${KDE4_SOLID_LIBS} ${KDE4_PHONON_LIBS} ${ASOUND_LIBRARY})

set_target_properties(kaudiodevicelist PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kaudiodevicelist  DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
	kaudiodevicelist_export.h
	audiodevice.h
	audiodeviceenumerator.h
	DESTINATION ${INCLUDE_INSTALL_DIR}/phonon)
install(FILES hardwaredatabase DESTINATION ${DATA_INSTALL_DIR}/libphonon)
