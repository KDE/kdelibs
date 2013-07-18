cmake_minimum_required(VERSION 2.8.11)

# This CMakeLists.txt is a "superbuild" for the libraries in tier1/ and tier2/.
# This means it can be used to build them all standalone one-by-one.
# This is useful to check whether they actually build and install standalone cleanly.
#
# To use it, run cmake pointing to kdelibs/superbuild/ as top level directory.
# Installation happens already during the build phase, this is different from
# normal builds.
# After running initially, no libraries are enabled.
# To do so, run cmake-gui, and check those libraries you want to build, e.g. "BUILD_karchive".
#
# For each enabled library, a subdirectory for building will be created.
# You can also change into that directory and build there like in a normal build dir
# (it actually is a normal build dir).
# When disabling a library, its builddir will be removed.
#
# If the library depends on other libraries of kdelibs, i.e. it is a tier2/ or tier3/ library,
# it will try to use them from the superbuild. If the respective library has not been
# enabled in the superbuild, it will try to use the one installed on the system.


include(SuperBuild.cmake)

sb_add_project(tier1/itemmodels )
sb_add_project(tier1/karchive )
sb_add_project(tier1/kcodecs )
sb_add_project(tier1/kconfig)
sb_add_project(tier1/kcoreaddons )
sb_add_project(tier1/kdbusaddons )
sb_add_project(tier1/kidletime )
sb_add_project(tier1/kjs )
sb_add_project(tier1/kplotting )
sb_add_project(tier1/kwidgetsaddons )
sb_add_project(tier1/kwindowsystem )
sb_add_project(tier1/solid )
sb_add_project(tier1/sonnet )
sb_add_project(tier1/threadweaver )

sb_add_project(tier2/kauth DEPENDS kcoreaddons)

sb_add_project(staging/kguiaddons DEPENDS kconfig)

sb_add_project(staging/kcrash         DEPENDS kcoreaddons kwindowsystem)
sb_add_project(staging/itemviews      DEPENDS kcoreaddons)
sb_add_project(staging/kconfigwidgets DEPENDS kconfig kwidgetsaddons)

if(SB_EVERYTHING_DISABLED)
  message(STATUS "All projects are disabled. You may consider running make edit_cache or cmake-gui to enable some.")
endif()
