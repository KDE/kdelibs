project( parseucd )

set( SOURCE_FILES
    parseucd/parseucd.cpp
)

include_directories(${X11_INCLUDE_DIR})

add_executable( parseucd ${SOURCE_FILES} )

target_link_libraries( parseucd
                       Qt5::Widgets
                       ${X11_LIBRARIES}
                     )
