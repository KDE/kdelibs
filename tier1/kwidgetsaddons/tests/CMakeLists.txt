
include(ECMMarkAsTest)

find_package(Qt5 5.2.0 CONFIG REQUIRED Test)

macro(kwidgetsaddons_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt5::Test KWidgetsAddons)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kwidgetsaddons_executable_tests(
  kanimatedbuttontest
  kcharselecttest
  kdatepicktest
  kdatetimeedittestapp
  kledtest
  kmessageboxtest
  kmessageboxwidtest
  kselectactiontest
  kseparatortest
  ksqueezedtextlabeltest
  ktitlewidgettest
  kfontrequestertest
  kpassworddialogtest
  keditlistwidgettest
  kratingwidgettest
  kactionselectortest
  kcolorcombotest
  kpixmapregionselectordialogtest
  kpassivepopuptest
  ktoolbarlabelactiontest
  kpagedialogtest
  kpagewidgettest
  kassistantdialogtest
  kmessagewidgettest
  knewpassworddialogtest
  kpixmapsequenceoverlaypaintertest
  kmimetypechoosertest
)

add_executable(kdatetabletest kdatetabletest.cpp ../src/kdatetable.cpp)
target_include_directories(kdatetabletest PRIVATE ../src)
target_link_libraries(kdatetabletest Qt5::Widgets)
ecm_mark_as_test(kdatetabletest)
