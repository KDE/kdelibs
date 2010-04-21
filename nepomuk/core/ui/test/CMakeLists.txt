set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${CMAKE_SOURCE_DIR}
  )

kde4_add_executable(ratingpaintertestapp TEST ratingpaintertestapp.cpp ratingpaintertestwidget.cpp)
target_link_libraries(ratingpaintertestapp
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  nepomuk
  )

kde4_add_executable(tagwidgetapp TEST tagwidgetapp.cpp tagwidgettest.cpp)
target_link_libraries(tagwidgetapp
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  nepomuk
  )
