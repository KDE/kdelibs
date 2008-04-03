set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_SOURCE_DIR}/includes)
kde4_add_executable(guitest TEST
   anglewidget.cpp
   audiooutputitem.cpp
   chapterwidget.cpp
   effectitem.cpp
   main.cpp
   mediaobjectitem.cpp
   mygraphicsscene.cpp
   navigationwidget.cpp
   pathitem.cpp
   titlewidget.cpp
   widgetrectitem.cpp
   )
target_link_libraries(guitest ${KDE4_PHONON_LIBS} ${KDE4_KIO_LIBS})
