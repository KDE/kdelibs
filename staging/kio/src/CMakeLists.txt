# Generate local forwarding headers

set(_INCDIR ${CMAKE_CURRENT_BINARY_DIR}/include/kio)
file(MAKE_DIRECTORY ${_INCDIR})

file(GLOB _KIOCOREHEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/core core/*.h)
#message(STATUS _KIOCOREHEADERS=${_KIOCOREHEADERS})
foreach(_KIOCOREHEADER ${_KIOCOREHEADERS})
    string(REGEX MATCH ".*_p\\.h" _ISPRIVATE ${_KIOCOREHEADER})
    #message(STATUS "_ISPRIVATE=${_ISPRIVATE} _KIOCOREHEADER=${_KIOCOREHEADER}")
    if (NOT _ISPRIVATE)
        if (NOT EXISTS ${_INCDIR}/${_KIOCOREHEADER})
            file(WRITE ${_INCDIR}/${_KIOCOREHEADER} "#include \"${CMAKE_CURRENT_SOURCE_DIR}/core/${_KIOCOREHEADER}\"\n")
        endif()
    endif()
endforeach()
if (NOT EXISTS ${_INCDIR}/kiocore_export.h)
    file(WRITE ${_INCDIR}/kiocore_export.h "#include \"${CMAKE_CURRENT_BINARY_DIR}/core/kiocore_export.h\"\n")
endif()

add_subdirectory(core)

file(GLOB _KIOWIDGETSHEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/widgets widgets/*.h)
#message(STATUS _KIOWIDGETSHEADERS=${_KIOWIDGETSHEADERS})
foreach(_KIOWIDGETSHEADER ${_KIOWIDGETSHEADERS})
   string(REGEX MATCH ".*_p\\.h" _ISPRIVATE ${_KIOWIDGETSHEADER})
   #message(STATUS "_ISPRIVATE=${_ISPRIVATE} _KIOWIDGETSHEADER=${_KIOWIDGETSHEADER}")
    if (NOT _ISPRIVATE)
       if (NOT EXISTS ${_INCDIR}/${_KIOWIDGETSHEADER})
          file(WRITE ${_INCDIR}/${_KIOWIDGETSHEADER} "#include \"${CMAKE_CURRENT_SOURCE_DIR}/widgets/${_KIOWIDGETSHEADER}\"\n")
        endif()
    endif()
endforeach()
if (NOT EXISTS ${_INCDIR}/kiowidgets_export.h)
    file(WRITE ${_INCDIR}/kiowidgets_export.h "#include \"${CMAKE_CURRENT_BINARY_DIR}/widgets/kiowidgets_export.h\"\n")
endif()

add_subdirectory(widgets)

if ( NOT WINCE )
    add_subdirectory(kssld)
endif ( NOT WINCE )

