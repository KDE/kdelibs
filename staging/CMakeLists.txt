find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

add_subdirectory(kservice)
add_subdirectory(ki18n) # must be before sonnet, kwidgets, etc.
add_subdirectory(sonnet)
add_subdirectory(kguiaddons)
add_subdirectory(kwidgets)
add_subdirectory(kio)
add_subdirectory(frameworkintegration)

add_subdirectory(kinterprocesswindowing)
add_subdirectory(kde4support)
add_subdirectory(kemoticons)
