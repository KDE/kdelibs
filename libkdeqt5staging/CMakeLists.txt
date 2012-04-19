cmake_minimum_required(VERSION 2.8.8)

project(kdeqt5staging)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt4 REQUIRED QtCore)

include(${QT_USE_FILE})

include(FeatureSummary)

# in case we are not build as part of the whole kdelibs
if (NOT TARGET inqt5)
  find_package(inqt5)
  # whatever works...
  include_directories(
      ${inqt5_SOURCE_DIR}/libinqt5/src/
      ${inqt5_BINARY_DIR}/libinqt5/src/
      ${inqt5_INCLUDE_DIR}
  )
  if (NOT INQT5_LIBRARY)
    set(INQT5_LIBRARY "${inqt5_LIBRARY}")
  endif ()
endif ()

add_subdirectory(src)
add_subdirectory(autotests)

