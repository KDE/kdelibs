project(kidletime)
cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${QT_INCLUDES}
        ${KDE4_INCLUDES}
        )

set(kidletime_SRCS
        kidletime.cpp
        abstractsystempoller.cpp
	widgetbasedpoller.cpp
        )

if (Q_WS_X11)
    set(kidletime_SRCS ${kidletime_SRCS} 
        xscreensaverbasedpoller.cpp
        xsyncbasedpoller.cpp )

    qt4_add_dbus_interface(kidletime_SRCS org.freedesktop.ScreenSaver.xml screensaver_interface )
elseif (Q_WS_MAC)
    set(kidletime_SRCS ${kidletime_SRCS} macpoller.cpp)
elseif (Q_WS_WIN)
    set(kidletime_SRCS ${kidletime_SRCS} windowspoller.cpp)
endif (Q_WS_X11)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

kde4_add_library(kidletime SHARED ${kidletime_SRCS})

target_link_libraries(kidletime ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

if (Q_WS_X11)
    target_link_libraries(kidletime Xtst Xss Xext X11)
elseif (Q_WS_MAC)
#    target_link_libraries(kidletime maclib)
elseif (Q_WS_WIN)
#    target_link_libraries(kidletime winlib)
endif (Q_WS_X11)


set_target_properties(kidletime PROPERTIES
        VERSION 1.0.0
        SOVERSION 1)
        #DEFINE_SYMBOL MAKE_KGLLIB_LIB)

install(FILES
        kidletime.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
        )

install(TARGETS kidletime ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(example)
