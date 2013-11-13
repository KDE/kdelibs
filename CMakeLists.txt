
set(reqd_cmake 2.8.12)
if (CMAKE_VERSION VERSION_LESS reqd_cmake)
  message(FATAL_ERROR "
  KDE Frameworks requires CMake 2.8.12 or later.

  http://thread.gmane.org/gmane.comp.programming.tools.cmake.devel/8114

  It is recommended that you build from git.

  http://community.kde.org/Frameworks/Building#CMake_from_Git
  ")
endif()
cmake_minimum_required(VERSION ${reqd_cmake})

project(kdelibs)

# Make CPack available to easy generate binary packages
include(CPack)

option(BUILD_TESTING "Build the testing tree." ON)
if(BUILD_TESTING)
    enable_testing()
endif()

add_subdirectory( tier1 )

# tier1/ already builds standalone, so it should not get any include dirs from here.

if(MSVC)
  # Only build-tested up to here.
  return()
endif()

add_subdirectory( tier2 )
add_subdirectory( tier3 )
add_subdirectory( tier4 )

add_subdirectory( interfaces  )

feature_summary(WHAT ALL
                INCLUDE_QUIET_PACKAGES
                FATAL_ON_MISSING_REQUIRED_PACKAGES
)

