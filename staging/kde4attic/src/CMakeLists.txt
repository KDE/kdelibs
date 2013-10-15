configure_file(config-kde4attic.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kde4attic.h )

# Configure checks for date/
include(date/ConfigureChecks.cmake)
configure_file(date/config-date.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/date/config-date.h)

# Configure checks for localization
include(localization/ConfigureChecks.cmake)
configure_file(localization/config-localization.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/localization/config-localization.h)

set(kde4attic_SRCS
  kdatecombobox.cpp
  kdatepicker.cpp
  kdatetable.cpp
  kdatetimeedit.cpp
  kdatetimewidget.cpp
  kdatewidget.cpp
  kgesture.cpp
  kgesturemap.cpp
  ktimecombobox.cpp
  ktimezonewidget.cpp
  ksslcertificatemanager.cpp
  ktcpsocket.cpp

  date/kcalendarera.cpp
  date/kcalendarsystem.cpp
  date/kcalendarsystemcoptic.cpp
  date/kcalendarsystemethiopian.cpp
  date/kcalendarsystemgregorian.cpp
  date/kcalendarsystemislamiccivil.cpp
  date/kcalendarsystemhebrew.cpp
  date/kcalendarsystemindiannational.cpp
  date/kcalendarsystemjalali.cpp
  date/kcalendarsystemjapanese.cpp
  date/kcalendarsystemjulian.cpp
  date/kcalendarsystemminguo.cpp
  date/kcalendarsystemthai.cpp
  date/kdatetime.cpp
  date/kdatetimeformatter.cpp
  date/kdatetimeparser.cpp
  date/kdayperiod.cpp
  date/klocalizeddate.cpp
  date/ktimezone.cpp
  date/ksystemtimezone.cpp
  date/ktzfiletimezone.cpp

  localization/klocale.cpp
  localization/klocale_kde.cpp
)

if (APPLE)
   LIST(APPEND kde4attic_SRCS
      localization/klocale_mac.cpp
   )

   set(kde4attic_OPTIONAL_LIBS ${CARBON_LIBRARY})
endif (APPLE)

if (WIN32)
   LIST(APPEND kde4attic_SRCS
      localization/klocale_win.cpp
      date/ktimezone_win.cpp
   )
endif (WIN32)

if (UNIX)
   LIST(APPEND kde4attic_SRCS
      localization/klocale_unix.cpp
   )
endif (UNIX)

set(kde4attic_UI
  kdatetimeedit.ui
)

qt5_wrap_ui(kde4attic_SRCS ${kde4attic_UI} )

add_library(KDE4Attic ${kde4attic_SRCS})
generate_export_header(KDE4Attic)
add_library(KF5::KDE4Attic ALIAS KDE4Attic)

target_include_directories(KDE4Attic PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/date
  ${CMAKE_CURRENT_BINARY_DIR}/localization
)
set(kde4attic_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/date
  ${CMAKE_CURRENT_SOURCE_DIR}/localization
)
target_include_directories(KDE4Attic PUBLIC "$<BUILD_INTERFACE:${kde4attic_INCLUDES}>")

target_link_libraries(KDE4Attic
  LINK_PUBLIC
    Qt5::PrintSupport
    Qt5::Network
    KCompletion
    XmlGui
  LINK_PRIVATE
    Qt5::Concurrent
    KF5::KI18n
    KUnitConversion
    KNotifications
    KF5::KWindowSystem
    KF5::KGuiAddons
    ${kde4attic_OPTIONAL_LIBS}
)

set_target_properties(KDE4Attic PROPERTIES VERSION   ${KDE4ATTIC_VERSION_STRING}
                                           SOVERSION ${KDE4ATTIC_SOVERSION}
)

install(TARGETS KDE4Attic EXPORT KDE4AtticTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kdatecombobox.h
  kdatepicker.h
  kdatetimeedit.h
  kdatetimewidget.h
  kdatewidget.h
  kgesture.h
  ktimecombobox.h
  ktimezonewidget.h
  ktcpsocket.h

  date/kcalendarsystem.h
  date/kdatetime.h
  date/klocalizeddate.h
  date/ksystemtimezone.h
  date/ktimezone.h
  date/ktzfiletimezone.h

  localization/klocale.h

  ${CMAKE_CURRENT_BINARY_DIR}/kde4attic_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

configure_file(localization/all_languages.desktop ${CMAKE_CURRENT_BINARY_DIR}/all_languages @ONLY)
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/all_languages DESTINATION ${LOCALE_INSTALL_DIR}  )
install( FILES localization/entry.desktop  DESTINATION  ${LOCALE_INSTALL_DIR}/en_US )
