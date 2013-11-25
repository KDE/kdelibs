remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)
include(ConfigureChecks.cmake) #configure checks for QFileSystemWatcher

find_package(Qt5 5.2.0 CONFIG REQUIRED Test)

macro(KCOREADDONS_EXECUTABLE_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        # TODO: split out the QtNetwork-dependent stuff into a separate addon?
        target_link_libraries(${_testname} KCoreAddons )
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

macro(KCOREADDONS_UNIT_TESTS)
    foreach(_testname ${ARGN})
        kcoreaddons_executable_tests(${_testname})
        target_link_libraries(${_testname} Qt5::Test)
        add_test(kcoreaddons-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

kcoreaddons_unit_tests(
    kaboutdatatest
    kautosavefiletest
    kcompositejobtest
    kjobtest
    kprocesstest
    kshareddatacachetest
    kshelltest
    kurlmimedatatest
    kstringhandlertest
)

set(KDIRWATCH_BACKENDS_TO_TEST Stat)#Stat is always compiled

if (HAVE_SYS_INOTIFY_H)
    list(APPEND KDIRWATCH_BACKENDS_TO_TEST INotify)
endif()

if (HAVE_FAM)
    list(APPEND KDIRWATCH_BACKENDS_TO_TEST Fam)
endif()

if (HAVE_QFILESYSTEMWATCHER)
    list(APPEND KDIRWATCH_BACKENDS_TO_TEST QFSWatch)
endif()

foreach(_backendName ${KDIRWATCH_BACKENDS_TO_TEST})
    string(TOLOWER ${_backendName} _lowercaseBackendName)
    set(BACKEND_TEST_TARGET kdirwatch_${_lowercaseBackendName}_unittest)
    add_executable(${BACKEND_TEST_TARGET} kdirwatch_unittest.cpp)
    target_link_libraries(${BACKEND_TEST_TARGET} Qt5::Test KCoreAddons)
    ecm_mark_as_test(${BACKEND_TEST_TARGET})
    add_test(${BACKEND_TEST_TARGET} ${BACKEND_TEST_TARGET})
    target_compile_definitions(${BACKEND_TEST_TARGET} PUBLIC -DKDIRWATCH_TEST_METHOD=\"${_backendName}\")
endforeach()
