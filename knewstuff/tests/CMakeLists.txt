project(knewstuff2tests)

include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_definitions(-DKNSSRCDIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\")
set(CMAKE_SKIP_RPATH TRUE)

##########################

set(knewstuff2_test_SRCS
   knewstuff2_test.cpp
)

kde4_automoc(${knewstuff2_test_SRCS})
kde4_add_executable(knewstuff2_test ${knewstuff2_test_SRCS})

target_link_libraries(knewstuff2_test ${KDE4_KDECORE_LIBS} knewstuff2)
#install(FILES knewstuff2_test.knsrc DESTINATION ${CONFIG_INSTALL_DIR})

##########################
 
set(knewstuff2_download_SRCS
   knewstuff2_download.cpp
)

kde4_automoc(${knewstuff2_download_SRCS})
kde4_add_executable(knewstuff2_download ${knewstuff2_download_SRCS})

target_link_libraries(knewstuff2_download ${KDE4_KDECORE_LIBS} knewstuff2)

##########################
 
set(knewstuff2_standard_SRCS
   knewstuff2_standard.cpp
)

kde4_automoc(${knewstuff2_standard_SRCS})
kde4_add_executable(knewstuff2_standard ${knewstuff2_standard_SRCS})

target_link_libraries(knewstuff2_standard ${KDE4_KDECORE_LIBS} knewstuff2)

##########################

set(kdxspreview_SRCS
   kdxspreview.cpp
)

#add_definitions(-DKNSSRCDIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\")
#set(CMAKE_SKIP_RPATH TRUE)

kde4_automoc(${kdxspreview_SRCS})
kde4_add_executable(kdxspreview ${kdxspreview_SRCS})

target_link_libraries(kdxspreview ${KDE4_KDECORE_LIBS} knewstuff2)
install(FILES kdxspreviewrc DESTINATION ${CONFIG_INSTALL_DIR})

