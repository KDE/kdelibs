cmake_minimum_required(VERSION 2.8.11)

include(SuperBuild.cmake)

sb_add_project(tier1/itemmodels )
sb_add_project(tier1/karchive )
sb_add_project(tier1/kcodecs )
sb_add_project(tier1/kcoreaddons )
sb_add_project(tier1/kdbusaddons )
sb_add_project(tier1/kidletime )
sb_add_project(tier1/kjs )
sb_add_project(tier1/kplotting )
sb_add_project(tier1/kwidgetsaddons )
sb_add_project(tier1/kwindowsystem )
sb_add_project(tier1/solid )
sb_add_project(tier1/threadweaver )

sb_add_project(tier2/kconfig DEPENDS kcoreaddons)
sb_add_project(tier2/kauth DEPENDS kcoreaddons)

sb_add_project(staging/kguiaddons DEPENDS kconfig)

if(SB_EVERYTHING_DISABLED)
  message(STATUS "All projects are disabled. You may consider running make edit_cache or cmake-gui to enable some.")
endif()

