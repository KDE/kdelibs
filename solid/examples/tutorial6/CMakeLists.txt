project(tutorial6)

# search KDE4 and use apply default (but optional) settings
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )
add_definitions(${QT_DEFINITIONS} ${QDBUS_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)

set(tutorial6_SRCS  tutorial6.cpp  webcamwatcher.cpp )

kde4_add_executable(tutorial6 ${tutorial6_SRCS})
target_link_libraries(tutorial6  ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} )

