project(tutorial2)

set(tutorial2_SRCS
 tutorial2.cpp
)

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
                     ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )
add_definitions(${QT_DEFINITIONS} ${QDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
link_directories(${KDE4_LIB_DIR})


kde4_add_executable(tutorial2 ${tutorial2_SRCS})

target_link_libraries(tutorial2  ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} )
