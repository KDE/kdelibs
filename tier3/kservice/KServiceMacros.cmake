#
# kservice_desktop_to_json(desktopfile [jsonfile])
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
#  kservice_desktop_to_json(plasma-dataengine-time.desktop)
#
# or:
#
#  kservice_desktop_to_json(plasma-dataengine-time.desktop my_output_file.json)
#
# WARNING: This macro runs desktoptojson at *build* time. This was
# necessary because the .json file file must be generated before moc
# is run, and there is currently no way to define a target as a
# dependency of the automoc target.

macro(kservice_desktop_to_json desktop)
    # replace file extension if second argument is empty
    set(json ${ARGV1})
    if(NOT json)
        string(REPLACE ".desktop" ".json" json ${desktop})
    endif()

    # find and run desktoptojson
    get_target_property(desktoptojson KF5::desktoptojson LOCATION)
    execute_process(
        COMMAND ${desktoptojson} -i ${desktop} -o ${CMAKE_CURRENT_BINARY_DIR}/${json}
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    if (NOT result EQUAL 0)
        message(FATAL_ERROR "Generating ${json} failed")
    endif()
endmacro(kservice_desktop_to_json)
