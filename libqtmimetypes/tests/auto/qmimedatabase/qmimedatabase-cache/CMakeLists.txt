set(SOURCES
	tst_qmimedatabase-cache.cpp
)

qt4_generate_moc(../tst_qmimedatabase.h moc_tst_qmimedatabase.cpp)

include_directories( ../../../../include/QtMimeTypes ../../../../src/mimetypes )

add_executable( tst_qmimedatabase-cache ${SOURCES} moc_tst_qmimedatabase.cpp)
add_test( tst_qmimedatabase-cache tst_qmimedatabase-cache )

set_target_properties(tst_qmimedatabase-cache PROPERTIES COMPILE_FLAGS -DSRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/../\\"")

target_link_libraries( tst_qmimedatabase-cache QtMimeTypes inqt5 ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY})

