project(kparts)

include_directories(${ki18n_BINARY_DIR}/src ${ki18n_SOURCE_DIR}/src)
include_directories(${KDE4_KIO_INCLUDES} ${kparts_BINARY_DIR})

add_subdirectory( tests )

if(HAVE_NEPOMUK)
    include_directories(${nepomuk_SOURCE_DIR} ${nepomuk_SOURCE_DIR}/core ${nepomuk_SOURCE_DIR}/types)
endif(HAVE_NEPOMUK)
if(Soprano_FOUND)
include_directories(${SOPRANO_INCLUDE_DIR})
endif(Soprano_FOUND)


########### next target ###############

set(kparts_LIB_SRCS
   part.cpp
   plugin.cpp
   partmanager.cpp
   mainwindow.cpp
   event.cpp
   browserextension.cpp
   factory.cpp
   historyprovider.cpp
   browserinterface.cpp
   browserrun.cpp
   browseropenorsavequestion.cpp
   statusbarextension.cpp
   scriptableextension.cpp
   textextension.cpp
   htmlextension.cpp
   fileinfoextension.cpp
   listingextension.cpp
   )

kde4_add_library(kparts ${LIBRARY_TYPE} ${kparts_LIB_SRCS})
generate_export_header(kparts)

target_link_libraries(kparts  ${KDE4_KDECORE_LIBS} kdeui kio ${KCoreAddons_LIBRARIES} kwidgets ki18n)
target_link_libraries(kparts  LINK_INTERFACE_LIBRARIES kio kdeui kdecore Qt5::Widgets kwidgets)
if(HAVE_NEPOMUK)
  target_link_libraries(kparts  nepomuk nepomukutils)
  target_link_libraries(kparts  LINK_INTERFACE_LIBRARIES nepomuk nepomukutils )
endif(HAVE_NEPOMUK)

set_target_properties(kparts PROPERTIES VERSION ${GENERIC_LIB_VERSION}
                                        SOVERSION ${GENERIC_LIB_SOVERSION}
                     )


install(TARGETS kparts EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES kpart.desktop krop.desktop krwp.desktop browserview.desktop   DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kparts_export.h part.h plugin.h partmanager.h mainwindow.h event.h
         browserextension.h factory.h historyprovider.h browserinterface.h
         browserrun.h statusbarextension.h browseropenorsavequestion.h
         scriptableextension.h textextension.h htmlextension.h fileinfoextension.h
         listingextension.h
         DESTINATION  ${INCLUDE_INSTALL_DIR}/kparts COMPONENT Devel )
