include_directories( ${KDE4_KIO_INCLUDES} .. )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

######### kmimefileparsertest ########

set(kmimefileparsertest_SRCS kmimefileparsertest.cpp ../kmimefileparser.cpp ../../kdecore/services/kmimemagicrule.cpp)

kde4_add_unit_test(kmimefileparsertest ${kmimefileparsertest_SRCS})

target_link_libraries(kmimefileparsertest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})

######### kmimeassociationstest ########

set(kmimeassociationstest_SRCS kmimeassociationstest.cpp ../kmimeassociations.cpp)

kde4_add_unit_test(kmimeassociationstest ${kmimeassociationstest_SRCS})

target_link_libraries(kmimeassociationstest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

