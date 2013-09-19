find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

add_subdirectory(itemviews)
add_subdirectory(kconfigwidgets)
add_subdirectory(kiconthemes)
add_subdirectory(kjobwidgets)
add_subdirectory(ktextwidgets)
add_subdirectory(xmlgui)

add_subdirectory(kwallet)
add_subdirectory(kservice)
add_subdirectory(kio)
add_subdirectory(frameworkintegration)

add_subdirectory(kinterprocesswindowing)
add_subdirectory(kde4attic)
add_subdirectory(kde4support)
add_subdirectory(kemoticons)

add_subdirectory(kbookmarks)
