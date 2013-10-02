project( KDELIBS_EXPERIMENTAL )
find_package(Qt5Quick)
find_package(X11)
set(HAVE_X11 ${X11_FOUND})

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

if(Qt5Quick_FOUND)
    add_subdirectory(kdeclarative)
endif()
