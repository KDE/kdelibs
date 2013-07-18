
set(kde4attic_SRCS
  kanimatedbutton.cpp
  kcharselect.cpp
  kcharselectdata.cpp
  kdatecombobox.cpp
  kdatepicker.cpp
  kdatetable.cpp
  kdatetimeedit.cpp
  kdatetimewidget.cpp
  kdatewidget.cpp
  kmultitabbar.cpp
  ktimecombobox.cpp
  ktimezonewidget.cpp
)

if(NOT WINCE)
set(kde4attic_SRCS
  ${kde4attic_SRCS}
  kdeprintdialog.cpp
)
endif(NOT WINCE)

if (X11_FOUND)
set(kde4attic_SRCS
  ${kde4attic_SRCS}
  kcupsoptionswidget_p.cpp
  kcupsoptionsjobwidget_p.cpp
  kcupsoptionspageswidget_p.cpp
  kcupsoptionssettingswidget_p.cpp )
endif (X11_FOUND)


set(kde4attic_UI
  kdatetimeedit.ui
)

if (X11_FOUND)
set(kde4attic_UI
  ${kde4attic_UI}
  kcupsoptionspageswidget.ui
  kcupsoptionsjobwidget.ui
)
endif (X11_FOUND)


kde4_add_ui_files(kde4attic_SRCS ${kde4attic_UI} )

add_library(KDE4Attic ${kde4attic_SRCS})
generate_export_header(KDE4Attic)

target_link_libraries(KDE4Attic LINK_PUBLIC Qt5::Widgets Qt5::PrintSupport kdeui kdecore KCompletion)

set_target_properties(KDE4Attic PROPERTIES VERSION   ${KDE4ATTIC_VERSION_STRING}
                                           SOVERSION ${KDE4ATTIC_SOVERSION}
)

install(TARGETS KDE4Attic EXPORT KDE4AtticTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 kdeprint_nup1.png
 kdeprint_nup2.png
 kdeprint_nup4.png
 kdeprint_nupother.png
 DESTINATION  ${DATA_INSTALL_DIR}/kdeui/pics )

install(FILES
  kanimatedbutton.h
  kcharselect.h
  kdatecombobox.h
  kdatepicker.h
  kdatetable.h
  kdatetimeedit.h
  kdatetimewidget.h
  kdatewidget.h
  kdeprintdialog.h
  kmultitabbar.h
  ktimecombobox.h
  ktimezonewidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/kde4attic_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install(FILES kcharselect-data  DESTINATION  ${DATA_INSTALL_DIR}/kcharselect)
