set(SOURCES
	tst_qmimetype.cpp
)

set(HEADERS
	tst_qmimetype.h
)

include_directories( ../../../include/QtMimeTypes ../../../src/mimetypes )

add_executable( tst_qmimetype ${SOURCES} )
add_test( tst_qmimetype tst_qmimetype )

find_package(Qt5Transitional REQUIRED Test)

target_link_libraries( tst_qmimetype QtMimeTypes ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})
