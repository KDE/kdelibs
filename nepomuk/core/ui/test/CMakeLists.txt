
include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${CMAKE_SOURCE_DIR}
  )

kde4_add_unit_test(ratingpaintertestapp ratingpaintertestapp.cpp ratingpaintertestwidget.cpp)
target_link_libraries(ratingpaintertestapp
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  nepomuk
  )
