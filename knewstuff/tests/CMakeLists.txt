project(knewstuff2tests)

include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR}/..
${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/../knewstuff2/
${CMAKE_CURRENT_SOURCE_DIR}/../knewstuff2/)

add_definitions(-DKNSSRCDIR=\\""${CMAKE_CURRENT_SOURCE_DIR}"\\" -DKNSBUILDDIR=\\""${CMAKE_CURRENT_BINARY_DIR}"\\")
set(CMAKE_SKIP_RPATH TRUE)

##########################

set(testTranslatable_SRCS
    testTranslatable.cpp
)

kde4_add_unit_test(testTranslatable ${testTranslatable_SRCS})

target_link_libraries(testTranslatable ${KDE4_KDECORE_LIBS} knewstuff2 ${QT_QTTEST_LIBRARY})

##########################

set(knewstuff2_test_SRCS
   knewstuff2_test.cpp
)

kde4_add_executable(knewstuff2_test TEST ${knewstuff2_test_SRCS})

target_link_libraries(knewstuff2_test ${KDE4_KDECORE_LIBS} knewstuff2)
#install(FILES knewstuff2_test.knsrc DESTINATION ${CONFIG_INSTALL_DIR})

##########################
 
set(knewstuff2_download_SRCS
   knewstuff2_download.cpp
)

kde4_add_executable(knewstuff2_download TEST ${knewstuff2_download_SRCS})

target_link_libraries(knewstuff2_download ${KDE4_KDECORE_LIBS} knewstuff2)

##########################
 
set(knewstuff2_standard_SRCS
   knewstuff2_standard.cpp
)

kde4_add_executable(knewstuff2_standard TEST ${knewstuff2_standard_SRCS})

target_link_libraries(knewstuff2_standard ${KDE4_KDECORE_LIBS} knewstuff2)

##########################
 
set(knewstuff2_cache_SRCS
   knewstuff2_cache.cpp
)

kde4_add_executable(knewstuff2_cache TEST ${knewstuff2_cache_SRCS})

target_link_libraries(knewstuff2_cache ${KDE4_KDECORE_LIBS} knewstuff2)

##########################

set(kdxspreview_SRCS
   kdxspreview.cpp
   kdxsview.cpp
)

#add_definitions(-DKNSSRCDIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\")
#set(CMAKE_SKIP_RPATH TRUE)

kde4_add_executable(kdxspreview TEST ${kdxspreview_SRCS})

target_link_libraries(kdxspreview ${KDE4_KDECORE_LIBS} knewstuff2)
install(FILES kdxspreviewrc DESTINATION ${CONFIG_INSTALL_DIR})

