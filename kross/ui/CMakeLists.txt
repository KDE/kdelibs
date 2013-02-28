include_directories(
    ${CMAKE_SOURCE_DIR}
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
    ${KAuth_INCLUDE_DIRS}
    ${ki18n_SOURCE_DIR}/src
    ${ki18n_BINARY_DIR}/src
)


set(krossui_LIB_SRCS
   model.cpp
   view.cpp
   plugin.cpp
#   guiclient.cpp
   )

kde4_add_library(krossui ${LIBRARY_TYPE} ${krossui_LIB_SRCS})
target_link_libraries(krossui krosscore ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KIO_LIBS} ${KCoreAddons_LIBRARIES} ki18n)
set_target_properties(krossui PROPERTIES VERSION ${GENERIC_LIB_VERSION}
                                         SOVERSION ${GENERIC_LIB_SOVERSION}
                     )

generate_export_header(krossui)

install(TARGETS krossui EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

#install(FILES guiclient.h model.h view.h DESTINATION ${INCLUDE_INSTALL_DIR}/kross/ui)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/krossui_export.h model.h view.h plugin.h DESTINATION ${INCLUDE_INSTALL_DIR}/kross/ui COMPONENT Devel)
