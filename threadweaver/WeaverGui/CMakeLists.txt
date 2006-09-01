include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/)

########### next target ###############

set(ThreadWeaverGui_LIB_SRCS
  WeaverThreadGrid.cpp
)

QT4_AUTOMOC(${ThreadWeaverGui_LIB_SRCS})

add_library(ThreadWeaverGui SHARED ${ThreadWeaverGui_LIB_SRCS})
target_link_libraries(ThreadWeaverGui ThreadWeaver ${QT_QTGUI_LIBRARY})
set_target_properties(ThreadWeaverGui PROPERTIES VERSION 4.2.0 SOVERSION 4)
install(TARGETS ThreadWeaverGui DESTINATION lib)

install(FILES
    WeaverThreadGrid.h
    DESTINATION include/threadweaver/
)
