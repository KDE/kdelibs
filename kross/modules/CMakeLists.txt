if( Qt5UiTools_FOUND )
	############################################
	# the forms module

	set(krossmoduleforms_SRCS form.cpp)
	add_library(krossmoduleforms MODULE ${krossmoduleforms_SRCS})

	target_link_libraries(krossmoduleforms Qt5::UiTools KParts ${KDE4_KFILE_LIBS} krosscore krossui KI18n)
	install(TARGETS krossmoduleforms DESTINATION ${PLUGIN_INSTALL_DIR})
endif( Qt5UiTools_FOUND )

############################################
# the kdetranslation module

set(krossmodulekdetranslation_SRCS translation.cpp)
add_library(krossmodulekdetranslation MODULE ${krossmodulekdetranslation_SRCS})

target_link_libraries(krossmodulekdetranslation KParts ${KDE4_CORE_LIBS} krosscore KI18n)
install(TARGETS krossmodulekdetranslation DESTINATION ${PLUGIN_INSTALL_DIR})
