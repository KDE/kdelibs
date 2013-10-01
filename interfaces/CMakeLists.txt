
project(interfaces)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" REQUIRED NO_MODULE COMPONENTS Core)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

find_package(Phonon4Qt5 4.6.60 NO_MODULE)
if (Phonon4Qt5_FOUND)
    add_subdirectory( kmediaplayer )
endif (Phonon4Qt5_FOUND)
add_subdirectory( terminal )
add_subdirectory( khexedit )
add_subdirectory( kimproxy )
add_subdirectory( kspeech )

########### install files ###############





