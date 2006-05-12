
if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${KDE4_KIO_INCLUDES} )

########### next target ###############

set(testresources_SRCS testresources.cpp )

kde4_automoc(${testresources_SRCS})

kde4_add_executable(testresources RUN_UNINSTALLED ${testresources_SRCS})

target_link_libraries(testresources  ${KDE4_KDECORE_LIBS} kresources)

########### next target ###############

set(idmappertest_SRCS idmappertest.cpp )

kde4_automoc(${idmappertest_SRCS})

kde4_add_executable(idmappertest RUN_UNINSTALLED ${idmappertest_SRCS})

target_link_libraries(idmappertest  ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kresources)

add_test(kresources-idmappertest ${EXECUTABLE_OUTPUT_PATH}/idmappertest)

endif(KDE4_BUILD_TESTS)

