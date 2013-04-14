
if( QT_QTDESIGNER_FOUND )
	############################################
	# the forms module

	set(krossmoduleforms_SRCS form.cpp)
	kde4_add_plugin(krossmoduleforms ${krossmoduleforms_SRCS})

	target_link_libraries(krossmoduleforms Qt5::UiTools ${KDE4_KPARTS_LIBS} ${KDE4_KFILE_LIBS} krosscore krossui ki18n)
	install(TARGETS krossmoduleforms DESTINATION ${PLUGIN_INSTALL_DIR})
endif( QT_QTDESIGNER_FOUND )

############################################
# the kdetranslation module

set(krossmodulekdetranslation_SRCS translation.cpp)
kde4_add_plugin(krossmodulekdetranslation ${krossmodulekdetranslation_SRCS})

target_link_libraries(krossmodulekdetranslation ${KDE4_KPARTS_LIBS} ${KDE4_CORE_LIBS} krosscore ki18n)
install(TARGETS krossmodulekdetranslation DESTINATION ${PLUGIN_INSTALL_DIR})
