if( Qt5UiTools_FOUND )
	############################################
	# the forms module

	set(krossmoduleforms_SRCS form.cpp)
    add_library(KrossModuleForms MODULE ${krossmoduleforms_SRCS})

    target_link_libraries(KrossModuleForms Qt5::UiTools KParts ${KDE4_KFILE_LIBS} KIOFileWidgets KrossCore KrossUi KI18n)
    install(TARGETS KrossModuleForms DESTINATION ${PLUGIN_INSTALL_DIR})
endif( Qt5UiTools_FOUND )

############################################
# the kdetranslation module

set(krossmodulekdetranslation_SRCS translation.cpp)
add_library(KrossModuleKdeTranslation MODULE ${krossmodulekdetranslation_SRCS})

target_link_libraries(KrossModuleKdeTranslation KParts ${KDE4_CORE_LIBS} KrossCore KI18n)
install(TARGETS KrossModuleKdeTranslation DESTINATION ${PLUGIN_INSTALL_DIR})
