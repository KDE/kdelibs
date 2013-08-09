kde4_add_unit_test(test_kconf_update
    test_kconf_update.cpp
    )

target_link_libraries(test_kconf_update
    Qt5::Test
    )

target_include_directories(test_kconf_update PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../..  ${CMAKE_CURRENT_BINARY_DIR}/..)

kde4_add_unit_test(test_kconfigutils
    test_kconfigutils.cpp
    ../kconfigutils.cpp
    )

target_link_libraries(test_kconfigutils
    KConfigCore
    Qt5::Test
    )
