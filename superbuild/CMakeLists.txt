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

# When adding a new framework, make sure it is listed *after* all its
# dependencies, otherwise superbuild will not pick up the standalone version of
# the framework build
sb_add_project(tier3/kconfigwidgets DEPENDS kauth kcoreaddons kcodecs kconfig kguiaddons ki18n kwidgetsaddons)
sb_add_project(tier3/kservice DEPENDS kauth kcoreaddons kcrash kdbusaddons ki18n)
sb_add_project(tier3/kiconthemes DEPENDS kconfigwidgets ki18n kwidgetsaddons itemviews)
sb_add_project(tier3/ktextwidgets DEPENDS sonnet ki18n kwidgetsaddons kservice kconfig kconfigwidgets kwindowsystem kiconthemes kcompletion)
sb_add_project(tier3/kunitconversion DEPENDS kconfig ki18n)
sb_add_project(tier3/xmlgui DEPENDS kconfig kconfigwidgets kwidgetsaddons itemviews ki18n kiconthemes ktextwidgets kwindowsystem)
sb_add_project(tier3/kjsembed DEPENDS kjs ki18n)
sb_add_project(tier3/kbookmarks DEPENDS kconfigwidgets kcoreaddons kiconthemes kwidgetsaddons xmlgui)
sb_add_project(tier3/kemoticons DEPENDS karchive kservice)
# Enable kross when kio and kparts are ready
#sb_add_project(tier3/kross DEPENDS ki18n kiconthemes kio kparts)
sb_add_project(tier3/kpty DEPENDS kcoreaddons ki18n)
sb_add_project(tier3/kdesu DEPENDS kcoreaddons kservice kpty)

if(SB_EVERYTHING_DISABLED)
  message(STATUS "All projects are disabled. You may consider running make edit_cache or cmake-gui to enable some.")
endif()
