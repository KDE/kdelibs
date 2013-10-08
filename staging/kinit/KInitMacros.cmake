set(_KINITDIR ${CMAKE_CURRENT_LIST_DIR})

macro(_FIND_KDEINIT_FILE OUTPUT_VAR INFIX)
    set(_KDE5INIT_DUMMY_FILENAME kde5init${INFIX}_dummy.cpp.in)
    if(kdelibs_SOURCE_DIR)
        set(${OUTPUT_VAR} "${CMAKE_SOURCE_DIR}/staging/kinit/${_KDE5INIT_DUMMY_FILENAME}")
    elseif(KInit_SOURCE_DIR)
        set(${OUTPUT_VAR} "${KInit_SOURCE_DIR}/${_KDE5INIT_DUMMY_FILENAME}")
    else()
        find_file(${OUTPUT_VAR} NAMES ${_KDE5INIT_DUMMY_FILENAME} HINTS ${_KINITDIR})
    endif()
endmacro()

function (KF5_ADD_KDEINIT_EXECUTABLE _target_NAME )
    cmake_parse_arguments(KF5_KDEINIT "NOGUI" "" "" ${ARGN} )
    set(_SRCS ${KF5_KDEINIT_UNPARSED_ARGUMENTS})

    _FIND_KDEINIT_FILE(_KDE5INIT_DUMMY_FILEPATH "")
    configure_file(${_KDE5INIT_DUMMY_FILEPATH} ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp)

    # under Windows, build a normal executable and additionally a dummy kdeinit5_foo.lib, whose only purpose on windows is to
    # keep the linking logic from the CMakeLists.txt on UNIX working (under UNIX all necessary libs are linked against the kdeinit
    # library instead against the executable, under windows we want to have everything in the executable, but for compatibility we have to
    # keep the library there-
    if(WIN32)
        if (MINGW)
            list(FIND _SRCS ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_res.o _res_position)
        else(MINGW)
            list(FIND _SRCS ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}.rc _res_position)
        endif(MINGW)
        if(NOT _res_position EQUAL -1)
            list(GET _SRCS ${_res_position} _resourcefile)
            list(REMOVE_AT _SRCS ${_res_position})
        endif(NOT _res_position EQUAL -1)

        _FIND_KDEINIT_FILE(_KDE5INIT_WIN32_DUMMY_FILEPATH "_win32lib")
        configure_file(${_KDE5INIT_WIN32_DUMMY_FILEPATH} ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_win32lib_dummy.cpp)
        add_library(kdeinit_${_target_NAME} STATIC ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_win32lib_dummy.cpp)

        add_executable(${_target_NAME} ${_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp ${_resourcefile})
    else(WIN32)

        add_library(kdeinit_${_target_NAME} SHARED ${_SRCS})

        if (Q_WS_MAC)
            list(FIND _SRCS *.icns _icon_position)
            if(NOT _res_position EQUAL -1)
                list(GET _SRCS ${_icon_position} _resourcefile)
            endif(NOT _res_position EQUAL -1)
        endif (Q_WS_MAC)
        add_executable(${_target_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${_target_NAME}_dummy.cpp ${_resourcefile})
    endif(WIN32)

    target_link_libraries(${_target_NAME} kdeinit_${_target_NAME})
    set_target_properties(kdeinit_${_target_NAME} PROPERTIES OUTPUT_NAME kdeinit5_${_target_NAME})

    if(KF5_KDEINIT_NOGUI)
        ecm_mark_nongui_executable(${_target_NAME})
    endif()
endfunction ()
