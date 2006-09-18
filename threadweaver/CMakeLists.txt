
include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

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

QT4_AUTOMOC(${ThreadWeaver_LIB_SRCS})

add_library(threadweaver SHARED ${ThreadWeaver_LIB_SRCS})
target_link_libraries(threadweaver ${QT_QTCORE_LIBRARY})
set_target_properties(threadweaver PROPERTIES VERSION 0.6.0 SOVERSION 0)
install(TARGETS threadweaver DESTINATION ${LIB_INSTALL_DIR})

install(FILES
    WeaverInterface.h
    ThreadWeaver .h
    WeaverImpl.h
    DebuggingAids.h
    Thread.h
    Job.h
    State.h
    StateImplementation.h
    InConstructionState.h
    WorkingHardState.h
    SuspendingState.h
    SuspendedState.h
    ShuttingDownState.h
    DestructedState.h
    WeaverObserver.h
    JobCollection.h
    JobSequence.h
    DependencyPolicy.h
    ResourceRestrictionPolicy.h
    QueuePolicy.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/threadweaver/
)
