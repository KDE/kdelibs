cmake_minimum_required(VERSION 2.8.12)

# This CMakeLists.txt is a "superbuild" for the libraries in tier1/ and tier2/.
# This means it can be used to build them all standalone one-by-one.
# This is useful to check whether they actually build and install standalone cleanly.

include(SuperBuild.cmake)

sb_add_project(tier1/itemmodels )
sb_add_project(tier1/itemviews )
sb_add_project(tier1/karchive )
sb_add_project(tier1/kcodecs )
sb_add_project(tier1/kconfig)
sb_add_project(tier1/kcoreaddons )
sb_add_project(tier1/kdbusaddons )
sb_add_project(tier1/kguiaddons )
sb_add_project(tier1/kidletime )
sb_add_project(tier1/kjs )
sb_add_project(tier1/kplotting )
sb_add_project(tier1/kwidgetsaddons )
sb_add_project(tier1/kwindowsystem )
sb_add_project(tier1/solid )
sb_add_project(tier1/sonnet )
sb_add_project(tier1/threadweaver )

sb_add_project(tier2/kauth DEPENDS kcoreaddons)
sb_add_project(tier2/kcompletion DEPENDS kconfig kwidgetsaddons)
sb_add_project(tier2/kcrash DEPENDS kcoreaddons kwindowsystem)
sb_add_project(tier2/kdoctools DEPENDS karchive)
sb_add_project(tier2/ki18n DEPENDS kjs)
sb_add_project(tier2/knotifications DEPENDS kwindowsystem)

sb_add_project(tier3/kconfigwidgets DEPENDS kauth kcoreaddons kcodecs kconfig kguiaddons ki18n kwidgetsaddons)

if(SB_EVERYTHING_DISABLED)
  message(STATUS "All projects are disabled. You may consider running make edit_cache or cmake-gui to enable some.")
endif()
