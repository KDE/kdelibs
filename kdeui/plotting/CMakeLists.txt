# for the kdeeduplot library we have different build options depending on
# whether we build with Qt4 only or with kde

# *** common variables ***

# sources for the kdeeduplot library
set( kdeeduplot_LIB_SRCS
     kplotobject.cpp
     kplotaxis.cpp
     kplotwidget.cpp
)

if (COMPILE_WITH_QT4_ONLY)

	# generate moc files automatically (requires that the
	# moc files are included in the cpp files)
	qt4_automoc(${kdeeduplot_LIB_SRCS})

	# add the definition for a QT4_ONLY build
	add_definitions( -DQT4_ONLY )
	
	# set include directories for a Qt4 build 
	include_directories( 
		${QT_INCLUDES}
		${CMAKE_BINARY_DIR}/kdeeduplot
	)

	# build the kdeeduplot library
	add_library(kdeeduplot SHARED ${kdeeduplot_LIB_SRCS})
	
	# link Qt4 libraries so that users of the kdeeduplot lib do
	# not have to worry about these dependencies
	target_link_libraries(kdeeduplot  ${QT_LIBRARIES} )
	
else (COMPILE_WITH_QT4_ONLY)

	# build test program in the tests subdir
	add_subdirectory( tests ) 

	# generate moc files automatically (requires that the
	# moc files are included in the cpp files)
	kde4_automoc(${kdeeduplot_LIB_SRCS})

	# build the kdeeduplot library as shared lib	
	kde4_add_library(kdeeduplot SHARED ${kdeeduplot_LIB_SRCS})
	
	# link kdeui library so that users of the kdeeduplot lib do
	# not have to worry about these dependencies
	target_link_libraries(kdeeduplot  ${KDE4_KDEUI_LIBS} )
	
	# set the properties for the build
	set_target_properties(kdeeduplot PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
	
	# install the binary library file into the KDE library directory
	install(TARGETS kdeeduplot  DESTINATION ${LIB_INSTALL_DIR} )

	# install the header files for the kdeeduplot library
	install(FILES
	   libkdeedu_plot_export.h
	   kplotobject.h
	   kplotaxis.h
	   kplotwidget.h DESTINATION ${INCLUDE_INSTALL_DIR}/libkdeedu
	)

endif (COMPILE_WITH_QT4_ONLY)
