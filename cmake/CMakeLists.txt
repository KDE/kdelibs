
#if(NOT AUTOMOC4_EXECUTABLE)
#   # this is just a fallback in case automoc4 from kdesupport isn't found, Alex
#   add_subdirectory(automoc)
#endif(NOT AUTOMOC4_EXECUTABLE)

if(NOT CROSS_COMPILATION_TOOLS_ONLY)
add_subdirectory(modules)
endif(NOT CROSS_COMPILATION_TOOLS_ONLY)

