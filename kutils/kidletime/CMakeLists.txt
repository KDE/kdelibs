project(kidle)
cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${QT_INCLUDES}
        ${KDE4_INCLUDES}
        )

set(kidle_SRCS
        kidle.cpp
        abstractsystempoller.cpp
        xsyncbasedpoller.cpp
        widgetbasedpoller.cpp
        )

qt4_add_dbus_interface(kidle_SRCS org.freedesktop.ScreenSaver.xml screensaver_interface )

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

kde4_add_library(kidle SHARED ${kidle_SRCS})

target_link_libraries(kidle ${KDE4_CORE_LIBRARY})
set_target_properties(kidle PROPERTIES
        VERSION 1.0.0
        SOVERSION 1)
        #DEFINE_SYMBOL MAKE_KGLLIB_LIB)

install(FILES
        kidle.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/kidle
        )
install(TARGETS kidle ${INSTALL_TARGETS_DEFAULT_ARGS})
