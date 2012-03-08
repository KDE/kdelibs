include_directories(
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_SOURCE_DIR}/widgets
 ${DBUSMENUQT_INCLUDE_DIR}
 ${CMAKE_BINARY_DIR}/staging/kwidgets/src
)

set(kwidgets_LIB_SRCS
 widgets/khbox.cpp
 widgets/kvbox.cpp
)

add_library(kwidgets ${LIBRARY_TYPE} ${kwidgets_LIB_SRCS})

generate_export_header(kwidgets)

target_link_libraries(kwidgets ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEUI_EXTRA_LIBS})

install(TARGETS kwidgets EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kwidgets_export.h
 widgets/khbox.h
 widgets/kvbox.h

 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
