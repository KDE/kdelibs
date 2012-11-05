set(SOURCES
	 main.cpp
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../include/QtMimeTypes ${CMAKE_CURRENT_SOURCE_DIR}/../../src/mimetypes)

add_executable( mimetypefinder ${SOURCES} )

target_link_libraries( mimetypefinder QtMimeTypes ${QT_QTCORE_LIBRARY} ${QT_QTMAIN_LIBRARY} )

