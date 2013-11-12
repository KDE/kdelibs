
project(interfaces)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" CONFIG REQUIRED Widgets DBus)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

find_package(Phonon4Qt5 4.6.60 NO_MODULE)
if (Phonon4Qt5_FOUND)
    add_subdirectory( kmediaplayer )
endif (Phonon4Qt5_FOUND)
add_subdirectory( kspeech )

########### install files ###############





