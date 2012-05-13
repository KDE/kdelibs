
########### next target ###############

set(ThreadWeaver_LIB_SRCS
    WeaverInterface.cpp
    ThreadWeaver.cpp
    WeaverImpl.cpp
    DebuggingAids.cpp
    Thread.cpp
    Job.cpp
    State.cpp
    StateImplementation.cpp
    InConstructionState.cpp
    WorkingHardState.cpp
    SuspendingState.cpp
    SuspendedState.cpp
    ShuttingDownState.cpp
    DestructedState.cpp
    WeaverObserver.cpp
    JobCollection.cpp
    JobSequence.cpp
    DependencyPolicy.cpp
    ResourceRestrictionPolicy.cpp
)

add_library(threadweaver ${ThreadWeaver_LIB_SRCS})
generate_export_header(threadweaver)
target_link_libraries(threadweaver ${QT_QTCORE_LIBRARY})

set_target_properties(threadweaver PROPERTIES
    VERSION ${ECM_VERSION_STRING}
    SOVERSION ${ECM_SOVERSION}
)
install(TARGETS threadweaver EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    WeaverInterface.h
    ThreadWeaver.h
    DebuggingAids.h
    Thread.h
    Job.h
    State.h
    WeaverObserver.h
    JobCollection.h
    JobSequence.h
    DependencyPolicy.h
    ResourceRestrictionPolicy.h
    QueuePolicy.h
    ${CMAKE_CURRENT_BINARY_DIR}/threadweaver_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver COMPONENT Devel
)
