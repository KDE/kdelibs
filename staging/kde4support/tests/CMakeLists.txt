
include(ECMMarkAsTest)

macro(KDE4SUPPORT_EXAMPLES)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} kde4support Qt5::Test Qt5::Network Qt5::Widgets kdecore kdeui ${KCoreAddons_LIBRARIES} ki18n)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kde4support_examples(
  kcolordlgtest
  kdialogtest
  kmdcodectest
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
)

if (X11_FOUND)
find_package(Qt5X11Extras 5.2.0 REQUIRED NO_MODULE)
kde4support_examples(
 kxerrorhandlertest
)

target_link_libraries(kxerrorhandlertest ${X11_X11_LIB})
endif (X11_FOUND)

