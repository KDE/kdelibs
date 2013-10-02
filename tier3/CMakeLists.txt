project(tier3)

add_subdirectory(ktextwidgets)
add_subdirectory(kunitconversion)
add_subdirectory(kparts)
add_subdirectory(kiconthemes)
add_subdirectory(xmlgui)
add_subdirectory(kprintutils)

if(Qt5WebKitWidgets_FOUND)
    add_subdirectory(kdewebkit)
endif()
