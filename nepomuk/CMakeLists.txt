project(nepomuk)

install(FILES
  nepomuk_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk
)

add_subdirectory(rcgen)
add_subdirectory(core)
