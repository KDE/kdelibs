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
sb_add_project(tier2/dnssd DEPENDS kconfig)
sb_add_project(tier2/kjobwidgets DEPENDS kcoreaddons kwidgetsaddons)
sb_add_project(tier2/kwallet DEPENDS kconfig kwindowsystem)

# When adding a new framework, make sure it is listed *after* all its
# dependencies, otherwise superbuild will not pick up the standalone version of
# the framework build
sb_add_project(tier3/kconfigwidgets DEPENDS kauth kcoreaddons kcodecs kconfig kdoctools kguiaddons ki18n kwidgetsaddons)
sb_add_project(tier3/kservice DEPENDS kauth kcoreaddons kcrash kdoctools kdbusaddons ki18n)
sb_add_project(tier3/kiconthemes DEPENDS kconfigwidgets ki18n kwidgetsaddons itemviews)
sb_add_project(tier3/ktextwidgets DEPENDS sonnet ki18n kwidgetsaddons kservice kconfig kconfigwidgets kwindowsystem kiconthemes kcompletion)
sb_add_project(tier3/kunitconversion DEPENDS kconfig ki18n)
sb_add_project(tier3/xmlgui DEPENDS kconfig kconfigwidgets kwidgetsaddons itemviews ki18n kiconthemes ktextwidgets kwindowsystem)
sb_add_project(tier3/kjsembed DEPENDS kdoctools kjs ki18n)
sb_add_project(tier3/kbookmarks DEPENDS kconfigwidgets kcoreaddons kiconthemes kwidgetsaddons xmlgui)
sb_add_project(tier3/kemoticons DEPENDS karchive kservice)
sb_add_project(tier3/kio DEPENDS kcoreaddons kdoctools kwidgetsaddons ki18n kservice karchive solid kjobwidgets kiconthemes kwindowsystem kcompletion kdoctools ktextwidgets xmlgui kbookmarks)
sb_add_project(tier3/kpty DEPENDS kcoreaddons ki18n)
sb_add_project(tier3/kdesu DEPENDS kcoreaddons kservice kpty)
sb_add_project(tier3/kparts DEPENDS kio xmlgui ktextwidgets ki18n knotifications kiconthemes)
sb_add_project(tier3/kross DEPENDS kdoctools ki18n kiconthemes kio kparts)
sb_add_project(tier3/kdewebkit DEPENDS kcoreaddons kwallet kio kparts)
sb_add_project(tier3/knotifyconfig DEPENDS kio ki18n knotifications)
sb_add_project(tier3/knewstuff DEPENDS ki18n itemviews karchive kiconthemes ktextwidgets xmlgui solid kbookmarks kio)
sb_add_project(tier3/kprintutils DEPENDS kcoreaddons ki18n kparts)
sb_add_project(tier3/kdeclarative DEPENDS kconfig ki18n kiconthemes kio)
sb_add_project(tier3/kdewidgets DEPENDS kio kdoctools kwidgetsaddons ki18n kiconthemes ktextwidgets itemviews)
sb_add_project(tier3/kinit DEPENDS kio kcrash kwindowsystem ki18n kservice kdbusaddons)
sb_add_project(tier3/kded DEPENDS kcoreaddons kdoctools kdbusaddons kservice kcrash kinit)
sb_add_project(tier3/kcmutils DEPENDS itemviews kconfigwidgets kcoreaddons ki18n kiconthemes kservice xmlgui)

sb_add_project(tier4/frameworkintegration DEPENDS ki18n kconfig kconfigwidgets kiconthemes knotifications)
sb_add_project(tier4/kde4support
    DEPENDS ki18n kguiaddons kwidgetsaddons kdbusaddons kconfig kconfigwidgets itemviews kiconthemes kunitconversion
            kwindowsystem kservice kcompletion ktextwidgets xmlgui kbookmarks kio kdoctools kcrash
)
sb_add_project(tier4/khtml
    DEPENDS karchive ki18n kcoreaddons kguiaddons kdbusaddons kservice kcrash kinit kjs kconfigwidgets itemviews kiconthemes
            knotifications kcompletion ktextwidgets xmlgui kparts kio kwallet)
