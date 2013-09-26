project(tier3)

add_subdirectory(ktextwidgets)
add_subdirectory(kunitconversion)
add_subdirectory(kparts)

if(Qt5WebKitWidgets_FOUND)
    add_subdirectory(kdewebkit)
endif()
