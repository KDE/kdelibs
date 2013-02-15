########### next target ###############

set(ThreadWeaver_LIB_SRCS
    Queue.cpp
    QueueAPI.cpp
    ThreadWeaver.cpp
    WeaverImpl.cpp
    DebuggingAids.cpp
    Thread.cpp
    Job.cpp
    Executor.cpp
    ExecuteWrapper.cpp
    State.cpp
    WeaverImplState.cpp
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

add_library(ThreadWeaver ${ThreadWeaver_LIB_SRCS})
generate_export_header(ThreadWeaver)
target_link_libraries(ThreadWeaver LINK_PUBLIC ${QT_QTCORE_LIBRARY})
target_include_directories(ThreadWeaver PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")

set_target_properties(ThreadWeaver PROPERTIES
    VERSION ${THREADWEAVER_VERSION_STRING}
    SOVERSION ${THREADWEAVER_SOVERSION}
)
install(TARGETS ThreadWeaver EXPORT ThreadWeaverTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

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
