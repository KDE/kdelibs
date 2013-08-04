#
# desktop_to_json(desktopfile [jsonfile])
#
# This macro uses desktoptojson to compile a json file, from a plugin
# description in a .desktop file. The generated file can be compiled
# into the plugin using the K_PLUGIN_FACTORY_WITH_JSON (cpp) macro.
#
# When jsonFile is omitted, the macro will replace the .desktop file
# extension of the first argument with .json and use that as output.
#
# Examples:
#
#  desktop_to_json(plasma-dataengine-time.desktop)
#
#  desktop_to_json(plasma-dataengine-time.desktop my_output_file.json)
#

macro(desktop_to_json desktop)

    # replace file extension if second argument is empty
    set(json ${ARGV1})
    if(NOT json)
        set(json)
        string(REPLACE ".desktop" ".json" json ${desktop})
    endif()

    # full paths to files
    set(_in ${CMAKE_CURRENT_SOURCE_DIR}/${desktop})
    set(_out ${CMAKE_CURRENT_BINARY_DIR}/${json})

    # add our json file as new target
    set(targ)
    string(REPLACE ".desktop" "Json" targ ${desktop})
    string(REPLACE "." "_" targ ${targ})
    add_custom_target(${targ} ALL DEPENDS ${_in})

    # ... and define it
    add_custom_command(
        TARGET ${targ} PRE_BUILD
        COMMAND KF5::desktoptojson -i ${_in} -o ${_out}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${_in}
        #COMMENT "Generating ${json}"
        )

endmacro(desktop_to_json)
