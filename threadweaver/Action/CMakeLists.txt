include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/)

########### next target ###############

set(ThreadWeaverAction_LIB_SRCS
    Action.cpp
    ActionJob_p.cpp
    ExecutePolicy.cpp
    JobsListPolicy.cpp
)

QT4_AUTOMOC(${ThreadWeaverAction_LIB_SRCS})

add_library(ThreadWeaverAction SHARED ${ThreadWeaverAction_LIB_SRCS})
target_link_libraries(ThreadWeaverAction ThreadWeaver)
set_target_properties(ThreadWeaverAction PROPERTIES VERSION 0.6.0 SOVERSION 0)
install(TARGETS ThreadWeaverAction DESTINATION lib)

install(FILES
    Action.h
    ExecutePolicy.h
    JobsListPolicy.h
    DESTINATION include/threadweaver/
)


#macro_additional_clean_files(${CMAKE_CURRENT_BINARY_DIR}/Job_moc.cpp ${CMAKE_CURRENT_BINARY_DIR}/Thread_moc.cpp)
