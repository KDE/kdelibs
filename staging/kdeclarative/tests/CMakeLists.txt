include(ECMAddTests)

ecm_add_tests(
    kdeclarativetest

    LINK_LIBRARIES Qt5::Quick KDeclarative KI18n KConfigCore
)
