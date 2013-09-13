
include(ECMMarkAsTest)

macro(KDE4SUPPORT_EXAMPLES)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} KDE4Support Qt5::Test Qt5::Network Qt5::Widgets KWidgetsAddons ${KCoreAddons_LIBRARIES} KI18n XmlGui KCompletion KIconThemes KIOCore)
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro()

kde4support_examples(
  kcolordlgtest
  kdebugtest_gui
  kdialogtest
  kmdcodectest
  kmessagetest
  kcmdlineargstest
  kdialogbuttonboxtest
  kmessageboxqueuedtest
  kprogressdialogtest
  khboxtest
  kshortcuttest
  kstatusbartest
  ksystemtraytest
  ktabwidgettest
  kfontdialogtest
  ktextbrowsertest
  keditlistboxtest
  knuminputtest
  kinputdialogtest
  kiopassdlgtest
  kmfitest
  kscantest
  netaccesstest
)

if (X11_FOUND)
  find_package(Qt5X11Extras 5.2.0 REQUIRED NO_MODULE)
  kde4support_examples( kxerrorhandlertest )

  target_link_libraries(kxerrorhandlertest ${X11_X11_LIB} Qt5::X11Extras)
endif ()

