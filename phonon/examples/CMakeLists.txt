include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_SOURCE_DIR}/includes)

# Examples don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

MACRO(PHONON_DECLARE_EXAMPLES)
    FOREACH(_examplename ${ARGN})
        kde4_automoc(${_examplename}.cpp)
        kde4_add_executable(${_examplename} ${_examplename}.cpp)
        target_link_libraries(${_examplename} ${KDE4_PHONON_LIBS})
    ENDFOREACH(_examplename)
ENDMACRO(PHONON_DECLARE_EXAMPLES)

PHONON_DECLARE_EXAMPLES(
    tutorial1
    tutorial2
    tutorial3
    tutorial4
    #crossfade
    #videoplayandforget
    #pcmoutput
    #videoplayback
    )
