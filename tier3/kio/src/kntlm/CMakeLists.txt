
set(kntlm_LIB_SRCS kntlm.cpp des.cpp )

add_library(kntlm ${kntlm_LIB_SRCS})
generate_export_header(kntlm)

target_link_libraries(kntlm KF5::CoreAddons)

set_target_properties(kntlm PROPERTIES VERSION ${KIO_VERSION_STRING} SOVERSION ${KIO_SOVERSION} )

install(TARGETS kntlm EXPORT KIOTargets  ${INSTALL_TARGETS_DEFAULT_ARGS})

#target_include_directories(kntlm PUBLIC
  #"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>" # kio_version.h
  #"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>" # kio/global.h
  #"$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/KDE>" # KIO/Job
#)

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kntlm_export.h kntlm.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

