
set(kntlm_LIB_SRCS kntlm.cpp des.cpp )

kde4_add_library(kntlm SHARED ${kntlm_LIB_SRCS})

target_link_libraries(kntlm  ${KDE4_KDECORE_LIBS} )

set_target_properties(kntlm PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS kntlm EXPORT kdelibsLibraryTargets  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES kntlm_export.h kntlm.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

