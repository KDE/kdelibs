include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_SOURCE_DIR}/includes)

# Examples don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

macro(PHONON_DECLARE_EXAMPLES)
    foreach(_examplename ${ARGN})
        kde4_add_executable(${_examplename} ${_examplename}.cpp)
        target_link_libraries(${_examplename} ${KDE4_PHONON_LIBS})
    endforeach(_examplename)
endmacro(PHONON_DECLARE_EXAMPLES)

phonon_declare_examples(
    tutorial2
    tutorial3
    tutorial4
    #videoplayer
    #crossfade
    #videoplayandforget
    #pcmoutput
    #videoplayback
    )

#target_link_libraries(videoplayer ${KDE4_KDEUI_LIBS})
