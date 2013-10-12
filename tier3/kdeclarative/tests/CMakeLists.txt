include(ECMMarkAsTest)

find_package(Qt5Test REQUIRED)

add_executable(kdeclarativetest kdeclarativetest.cpp)
ecm_mark_as_test(kdeclarativetest)
target_link_libraries(kdeclarativetest Qt5::Quick Qt5::Test KDeclarative KF5::KI18n KConfigCore)
