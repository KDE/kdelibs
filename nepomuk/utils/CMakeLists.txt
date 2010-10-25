project(nepomuk_utils)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/..
)

set(nepomukutils_SRCS
  utils.cpp
)

kde4_add_library(nepomukutils ${LIBRARY_TYPE} ${nepomukutils_SRCS})

target_link_libraries(nepomukutils
  ${SOPRANO_LIBRARIES}
  ${QT_QTCORE_LIBRARY}
  nepomuk
  nepomukquery
  kdecore
)

set_target_properties(nepomukutils PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS nepomukutils EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  utils.h
  nepomukutils_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk COMPONENT Devel
)
