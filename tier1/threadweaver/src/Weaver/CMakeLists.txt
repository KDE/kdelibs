########### next target ###############
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

set(ThreadWeaver_LIB_SRCS
    Queue.cpp
    QueueStream.cpp
    QueueAPI.cpp
    Exception.cpp
    Weaver.cpp
    WeaverImpl.cpp
    DebuggingAids.cpp
    Thread.cpp
    Job.cpp
    IdDecorator.cpp
    QObjectDecorator.cpp
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
    Dependency.cpp
    ResourceRestrictionPolicy.cpp
)

add_library(ThreadWeaver ${ThreadWeaver_LIB_SRCS})
generate_export_header(ThreadWeaver)
add_library(KF5::ThreadWeaver ALIAS ThreadWeaver)
target_link_libraries(ThreadWeaver PUBLIC Qt5::Core)
target_include_directories(ThreadWeaver
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>"
  INTERFACE
    "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/threadweaver>"
)

set_target_properties(ThreadWeaver PROPERTIES
    VERSION ${THREADWEAVER_VERSION_STRING}
    SOVERSION ${THREADWEAVER_SOVERSION}
)
install(TARGETS ThreadWeaver EXPORT ThreadWeaverTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ThreadWeaver.h
    QueueStream.h
    Queueing.h
    Exception.h
    WeaverInterface.h
    Weaver.h
    DebuggingAids.h
    Thread.h
    JobInterface.h
    Job.h
    IdDecorator.h
    QObjectDecorator.h
    Lambda.h
    State.h
    WeaverObserver.h
    JobCollection.h
    JobSequence.h
    Dependency.h
    DependencyPolicy.h
    ResourceRestrictionPolicy.h
    Queue.h
    QueueInterface.h
    QueuePolicy.h
    JobPointer.h
    ManagedJobPointer.h
    ${CMAKE_CURRENT_BINARY_DIR}/threadweaver_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver COMPONENT Devel
)
