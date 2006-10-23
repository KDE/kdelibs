

########### next target ###############

set(kdeeduplot_LIB_SRCS
   kplotobject.cpp
   kplotaxis.cpp
   kplotwidget.cpp
)

kde4_automoc(${kdeeduplot_LIB_SRCS})

kde4_add_library(kdeeduplot SHARED ${kdeeduplot_LIB_SRCS})

target_link_libraries(kdeeduplot  ${KDE4_KDEUI_LIBS} )

set_target_properties(kdeeduplot PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install(TARGETS kdeeduplot  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install(FILES
   libkdeedu_plot_export.h
   kplotobject.h
   kplotaxis.h
   kplotwidget.h DESTINATION ${INCLUDE_INSTALL_DIR}/libkdeedu
)


