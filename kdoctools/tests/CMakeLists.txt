add_test(
  NAME meinproc_template
  COMMAND meinproc4 ${CMAKE_CURRENT_SOURCE_DIR}/../template.docbook
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
