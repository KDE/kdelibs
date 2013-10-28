
# build the analyzer as a module
add_library(dummy MODULE dummyanalyzers.cpp)

# link with the required libraries
target_link_libraries(dummy ${STRIGI_STREAMANALYZER_LIBRARY})

# adjust the name so the module is recognized as a strigi plugin
set_target_properties(dummy PROPERTIES
    PREFIX strigiea_)
