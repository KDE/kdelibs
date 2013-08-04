

macro(DESKTOP_TO_JSON _input)
    set(_output)
    set(targ)
    string(REPLACE ".desktop" ".json" _output ${_input})
    string(REPLACE ".desktop" "Json" targ ${_input})

    set(_in ${CMAKE_CURRENT_SOURCE_DIR}/${_input})
    set(_out ${CMAKE_CURRENT_BINARY_DIR}/${_output})

    add_custom_target(${targ} ALL DEPENDS ${_in})
    add_custom_command(
        TARGET ${targ} PRE_BUILD
        COMMAND KF5::desktoptojson -i ${_in} -o ${_out}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${_in}
        COMMENT "\n\nGenerating ${_output}"
        )
endmacro(DESKTOP_TO_JSON)
