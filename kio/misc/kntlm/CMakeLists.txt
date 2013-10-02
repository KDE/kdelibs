
set(kntlm_LIB_SRCS kntlm.cpp des.cpp )

add_library(kntlm ${kntlm_LIB_SRCS})
generate_export_header(kntlm)

target_link_libraries(kntlm ${KCoreAddons_LIBRARIES})

set_target_properties(kntlm PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS kntlm EXPORT kdelibsLibraryTargets  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kntlm_export.h kntlm.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

