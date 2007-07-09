include_directories(${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/Weaver/ ${CMAKE_SOURCE_DIR}/Experimental/ ${KDE4_INCLUDES})
include_directories( ../../Weaver/ )
include_directories( ../../Experimental/ )

set(SMIV_SRCS
    SMIV.cpp
    SMIVItem.cpp
    SMIVModel.cpp
    SMIVView.cpp
    SMIVItemDelegate.cpp
    QImageLoaderJob.cpp
    ComputeThumbNailJob.cpp
)

set( SMIV_SRCS_UI
    SMIVBase.ui
)

qt4_wrap_ui(SMIV_SRCS ${SMIV_SRCS_UI} )


add_executable(SMIV ${SMIV_SRCS})

target_link_libraries(SMIV ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} threadweaver ThreadWeaverExperimental)

install(TARGETS SMIV DESTINATION ${BIN_INSTALL_DIR} )
