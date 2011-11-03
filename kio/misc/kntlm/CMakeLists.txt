include_directories(
  ${CMAKE_SOURCE_DIR}/tier1/libkcoreaddons/src/randomness
  ${CMAKE_BINARY_DIR}/tier1/libkcoreaddons/src
)

set(kntlm_LIB_SRCS kntlm.cpp des.cpp )

kde4_add_library(kntlm ${LIBRARY_TYPE} ${kntlm_LIB_SRCS})

target_link_libraries(kntlm  ${KDE4_KDECORE_LIBS} kcoreaddons)

set_target_properties(kntlm PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS kntlm  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES kntlm_export.h kntlm.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

