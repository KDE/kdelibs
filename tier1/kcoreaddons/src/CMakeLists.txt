# Configure checks for the caching subdir
include(CheckIncludeFiles)
check_include_files("sys/types.h;sys/mman.h" HAVE_SYS_MMAN_H)
configure_file(caching/config-caching.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-caching.h)

set(kcoreaddons_OPTIONAL_SRCS )
set(kcoreaddons_OPTIONAL_LIBS )

if (FAM_FOUND)
   include_directories(${FAM_INCLUDE_DIR})
   set(kcoreaddons_OPTIONAL_LIBS ${kcoreaddons_OPTIONAL_LIBS} ${FAM_LIBRARIES})
endif ()

if(NOT WIN32)
    set(kcoreaddons_OPTIONAL_SRCS caching/kshareddatacache.cpp)
    set(kcoreaddons_OPTIONAL_LIBS ${kcoreaddons_OPTIONAL_LIBS} ${CMAKE_THREAD_LIBS_INIT})

    set_source_files_properties(caching/kshareddatacache.cpp
        PROPERTIES COMPILE_FLAGS -fexceptions)

else(NOT WIN32)
    set(kcoreaddons_OPTIONAL_SRCS
        caching/kshareddatacache_win.cpp
        io/kdirwatch_win.cpp
    )
endif(NOT WIN32)

if (WIN32)
    set(kcoreaddons_OPTIONAL_SRCS
        ${kcoreaddons_OPTIONAL_SRCS}
        io/klockfile_win.cpp
        text/kmacroexpander_win.cpp
    )
endif ()

if (UNIX)
    set(kcoreaddons_OPTIONAL_SRCS
        ${kcoreaddons_OPTIONAL_SRCS}
        io/klockfile_unix.cpp
        text/kmacroexpander_unix.cpp
    )
endif ()

set(libkcoreaddons_SRCS
    kaboutdata.cpp
    io/kautosavefile.cpp
    io/kdirwatch.cpp
    io/kfilesystemtype_p.cpp
    io/kbackup.cpp
    io/kurlmimedata.cpp
    jobs/kcompositejob.cpp
    jobs/kjob.cpp
    jobs/kjobtrackerinterface.cpp
    jobs/kjobuidelegate.cpp
    randomness/krandom.cpp
    randomness/krandomsequence.cpp
    text/kmacroexpander.cpp
    text/kstringhandler.cpp
    ${kcoreaddons_OPTIONAL_SRCS}
)


set(kcoreaddons_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}/io/
    ${CMAKE_CURRENT_SOURCE_DIR}/io/
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/
    ${CMAKE_CURRENT_SOURCE_DIR}/randomness/
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/
    ${CMAKE_CURRENT_SOURCE_DIR}/text/
    ${CMAKE_CURRENT_SOURCE_DIR}/caching/
)

add_library(KCoreAddons ${libkcoreaddons_SRCS})
generate_export_header(KCoreAddons)

target_include_directories(KCoreAddons PUBLIC "$<BUILD_INTERFACE:${kcoreaddons_INCLUDE_DIRS}>")

target_link_libraries(KCoreAddons
  LINK_PUBLIC Qt5::Core ${kcoreaddons_OPTIONAL_LIBS}
  LINK_PRIVATE ${kdeqt5staging_LIBRARIES}
)
if(NOT WIN32)
    target_link_libraries(KCoreAddons LINK_PRIVATE pthread)
endif(NOT WIN32)

set_target_properties(KCoreAddons PROPERTIES VERSION   ${KCOREADDONS_VERSION_STRING}
                                             SOVERSION ${KCOREADDONS_SOVERSION}
)

install(TARGETS KCoreAddons EXPORT KCoreAddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kaboutdata.h
    caching/kshareddatacache.h
    io/kautosavefile.h
    io/kdirwatch.h
    io/klockfile.h
    io/kbackup.h
    io/kurlmimedata.h
    jobs/kcompositejob.h
    jobs/kcompositejob_p.h
    jobs/kjob.h
    jobs/kjob_p.h
    jobs/kjobtrackerinterface.h
    jobs/kjobuidelegate.h
    randomness/krandom.h
    randomness/krandomsequence.h
    text/kmacroexpander.h
    text/kstringhandler.h
    ${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
