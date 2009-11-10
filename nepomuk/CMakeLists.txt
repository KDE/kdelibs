project(nepomuk)

install(FILES
  nepomuk_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk COMPONENT Devel
)

add_subdirectory(rcgen)
add_subdirectory(core)
add_subdirectory(query)
add_subdirectory(cmake/modules)
