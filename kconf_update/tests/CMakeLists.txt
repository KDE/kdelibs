kde4_add_unit_test(test_kconf_update
    test_kconf_update.cpp
    )

target_link_libraries(test_kconf_update
    ${KDE4_KDECORE_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    )
