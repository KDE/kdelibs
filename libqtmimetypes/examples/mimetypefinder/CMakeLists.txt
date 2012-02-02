set(SOURCES
	 main.cpp
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../include/QtMimeTypes)

add_executable( mimetypefinder ${SOURCES} )

target_link_libraries( mimetypefinder QtMimeTypes ${QT_QTCORE_LIBRARY} )

