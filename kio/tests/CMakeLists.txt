# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# for <kio/global.h>
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########## ktradertest - which is installed

kde4_add_executable(ktradertest NOGUI ktradertest.cpp)

target_link_libraries(ktradertest ${KDE4_KIO_LIBS})

install(TARGETS ktradertest DESTINATION ${BIN_INSTALL_DIR} )

if(KDE4_BUILD_TESTS)

########### next target ###############

set(ksycocatest_SRCS ksycocatest.cpp)

kde4_automoc(${ksycocatest_SRCS})

kde4_add_executable(ksycocatest NOGUI RUN_UNINSTALLED ${ksycocatest_SRCS})

target_link_libraries(ksycocatest ${KDE4_KIO_LIBS})


########### next target ###############

set(getalltest_SRCS getalltest.cpp)

kde4_automoc(${getalltest_SRCS})

kde4_add_executable(getalltest NOGUI RUN_UNINSTALLED ${getalltest_SRCS})

target_link_libraries(getalltest ${KDE4_KIO_LIBS})


########### next target ###############

set(kruntest_SRCS kruntest.cpp)

kde4_automoc(${kruntest_SRCS})

kde4_add_executable(kruntest RUN_UNINSTALLED ${kruntest_SRCS})

target_link_libraries(kruntest ${KDE4_KIO_LIBS})


########### next target ###############

set(ktartest_SRCS ktartest.cpp)

kde4_automoc(${ktartest_SRCS})

kde4_add_executable(ktartest NOGUI RUN_UNINSTALLED ${ktartest_SRCS})

target_link_libraries(ktartest ${KDE4_KIO_LIBS})


########### next target ###############

set(kziptest_SRCS kziptest.cpp)

kde4_automoc(${kziptest_SRCS})

kde4_add_executable(kziptest NOGUI RUN_UNINSTALLED ${kziptest_SRCS})

target_link_libraries(kziptest ${KDE4_KIO_LIBS})

########### next target ###############

set(karchivetest_SRCS karchivetest.cpp)

kde4_automoc(${karchivetest_SRCS})

kde4_add_executable(karchivetest NOGUI RUN_UNINSTALLED ${karchivetest_SRCS})

target_link_libraries(karchivetest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kio-karchivetest ${EXECUTABLE_OUTPUT_PATH}/karchivetest)

########### next target ###############

set(kziptest_SRCS kziptest.cpp)

kde4_automoc(${kziptest_SRCS})

kde4_add_executable(kziptest NOGUI RUN_UNINSTALLED ${kziptest_SRCS})

target_link_libraries(kziptest ${KDE4_KIO_LIBS})

########### next target ###############

set(kioslavetest_SRCS kioslavetest.cpp)

kde4_automoc(${kioslavetest_SRCS})

kde4_add_executable(kioslavetest NOGUI RUN_UNINSTALLED ${kioslavetest_SRCS})

target_link_libraries(kioslavetest ${KDE4_KIO_LIBS})


########### next target ###############

set(kdirwatchtest_SRCS kdirwatchtest.cpp)

kde4_automoc(${kdirwatchtest_SRCS})

kde4_add_executable(kdirwatchtest RUN_UNINSTALLED ${kdirwatchtest_SRCS})

target_link_libraries(kdirwatchtest ${KDE4_KIO_LIBS})


########### next target ###############

set(speed_SRCS speed.cpp)

kde4_automoc(${speed_SRCS})

kde4_add_executable(speed RUN_UNINSTALLED ${speed_SRCS})

target_link_libraries(speed ${KDE4_KIO_LIBS})



########### next target ###############

set(kdefaultprogresstest_SRCS kdefaultprogresstest.cpp)

kde4_automoc(${kdefaultprogresstest_SRCS})

kde4_add_executable(kdefaultprogresstest RUN_UNINSTALLED ${kdefaultprogresstest_SRCS})

target_link_libraries(kdefaultprogresstest ${KDE4_KIO_LIBS})


########### next target ###############

set(kmimemagictest_SRCS kmimemagictest.cpp)

kde4_automoc(${kmimemagictest_SRCS})

kde4_add_executable(kmimemagictest NOGUI RUN_UNINSTALLED ${kmimemagictest_SRCS})

target_link_libraries(kmimemagictest ${KDE4_KIO_LIBS})


########### next target ###############

set(kiopassdlgtest_SRCS kiopassdlgtest.cpp)

kde4_automoc(${kiopassdlgtest_SRCS})

kde4_add_executable(kiopassdlgtest RUN_UNINSTALLED ${kiopassdlgtest_SRCS})

target_link_libraries(kiopassdlgtest ${KDE4_KIO_LIBS})


########### next target ###############

set(kscantest_SRCS kscantest.cpp)

kde4_automoc(${kscantest_SRCS})

kde4_add_executable(kscantest RUN_UNINSTALLED ${kscantest_SRCS})

target_link_libraries(kscantest ${KDE4_KIO_LIBS})


########### next target ###############

set(safesitetest_SRCS safesitetest.cpp)

kde4_automoc(${safesitetest_SRCS})

kde4_add_executable(safesitetest RUN_UNINSTALLED ${safesitetest_SRCS})

target_link_libraries(safesitetest ${KDE4_KIO_LIBS})


########### next target ###############

set(kdirlistertest_SRCS kdirlistertest.cpp)

kde4_automoc(${kdirlistertest_SRCS})

kde4_add_executable(kdirlistertest RUN_UNINSTALLED ${kdirlistertest_SRCS})

target_link_libraries(kdirlistertest ${KDE4_KIO_LIBS})


########### next target ###############

set(previewtest_SRCS previewtest.cpp)

kde4_automoc(${previewtest_SRCS})

kde4_add_executable(previewtest RUN_UNINSTALLED ${previewtest_SRCS})

target_link_libraries(previewtest ${KDE4_KIO_LIBS})


########### next target ###############

set(kionetrctest_SRCS kionetrctest.cpp)

kde4_automoc(${kionetrctest_SRCS})

kde4_add_executable(kionetrctest RUN_UNINSTALLED ${kionetrctest_SRCS})

target_link_libraries(kionetrctest ${KDE4_KIO_LIBS})


########### next target ###############

set(metatest_SRCS metatest.cpp)

kde4_automoc(${metatest_SRCS})

kde4_add_executable(metatest RUN_UNINSTALLED ${metatest_SRCS})

target_link_libraries(metatest ${KDE4_KIO_LIBS})


########### next target ###############

set(kmimefromext_SRCS kmimefromext.cpp)

kde4_automoc(${kmimefromext_SRCS})

kde4_add_executable(kmimefromext NOGUI RUN_UNINSTALLED ${kmimefromext_SRCS})

target_link_libraries(kmimefromext ${KDE4_KIO_LIBS})


########### next target ###############

set(kpropsdlgtest_SRCS kpropsdlgtest.cpp)

kde4_automoc(${kpropsdlgtest_SRCS})

kde4_add_executable(kpropsdlgtest RUN_UNINSTALLED ${kpropsdlgtest_SRCS})

target_link_libraries(kpropsdlgtest ${KDE4_KIO_LIBS})


########### next target ###############

set(kmfitest_SRCS kmfitest.cpp)

kde4_automoc(${kmfitest_SRCS})

kde4_add_executable(kmfitest RUN_UNINSTALLED ${kmfitest_SRCS})

target_link_libraries(kmfitest ${KDE4_KIO_LIBS})


########### next target ###############

set(dataprotocoltest_SRCS dataprotocoltest.cpp)

kde4_automoc(${dataprotocoltest_SRCS})

kde4_add_executable(dataprotocoltest RUN_UNINSTALLED ${dataprotocoltest_SRCS})

target_link_libraries(dataprotocoltest ${KDE4_KIO_LIBS})


########### next target ###############

set(kprotocolinfotest_SRCS kprotocolinfotest.cpp)

kde4_automoc(${kprotocolinfotest_SRCS})
kde4_add_executable(kprotocolinfotest RUN_UNINSTALLED ${kprotocolinfotest_SRCS})

target_link_libraries(kprotocolinfotest ${KDE4_KIO_LIBS})


########### next target ###############

set(ksycocaupdatetest_SRCS ksycocaupdatetest.cpp)

kde4_automoc(${ksycocaupdatetest_SRCS})

kde4_add_executable(ksycocaupdatetest RUN_UNINSTALLED ${ksycocaupdatetest_SRCS})

target_link_libraries(ksycocaupdatetest ${KDE4_KIO_LIBS})


########### next target ###############

set(netaccesstest_SRCS netaccesstest.cpp)

kde4_automoc(${netaccesstest_SRCS})

kde4_add_executable(netaccesstest RUN_UNINSTALLED ${netaccesstest_SRCS})

target_link_libraries(netaccesstest ${KDE4_KIO_LIBS})


########### next target ###############

set(jobtest_SRCS jobtest.cpp)

kde4_automoc(${jobtest_SRCS})

kde4_add_executable(jobtest RUN_UNINSTALLED ${jobtest_SRCS})

target_link_libraries(jobtest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY} )

# Seems to trigger a ctest problem; jobtest finishes and ctest waits for ever
#add_test(kio-jobtest ${EXECUTABLE_OUTPUT_PATH}/jobtest)


########### next target ###############

set(kurlcompletiontest_SRCS kurlcompletiontest.cpp)

kde4_automoc(${kurlcompletiontest_SRCS})

kde4_add_executable(kurlcompletiontest RUN_UNINSTALLED ${kurlcompletiontest_SRCS})

target_link_libraries(kurlcompletiontest ${KDE4_KIO_LIBS})


########### next target ###############

set(kmimetypetest_SRCS kmimetypetest.cpp)

kde4_automoc(${kmimetypetest_SRCS})

kde4_add_executable(kmimetypetest NOGUI RUN_UNINSTALLED ${kmimetypetest_SRCS})

target_link_libraries(kmimetypetest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY} )

add_test(kio-kmimetypetest ${EXECUTABLE_OUTPUT_PATH}/kmimetypetest)


########### next target ###############

set(kbookmarktest_SRCS kbookmarktest.cpp)

kde4_automoc(${kbookmarktest_SRCS})

kde4_add_executable(kbookmarktest RUN_UNINSTALLED ${kbookmarktest_SRCS})

target_link_libraries(kbookmarktest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY} )

add_test(kio-kbookmarktest ${EXECUTABLE_OUTPUT_PATH}/kbookmarktest)

########### next target ###############

set(kacltest_SRCS kacltest.cpp)

kde4_automoc(${kacltest_SRCS})

kde4_add_executable(kacltest RUN_UNINSTALLED ${kacltest_SRCS})

target_link_libraries(kacltest ${KDE4_KIO_LIBS})


########### next target ###############

set(kdirwatchtest_gui_SRCS kdirwatchtest_gui.cpp)

kde4_automoc(${kdirwatchtest_gui_SRCS})

kde4_add_executable(kdirwatchtest_gui RUN_UNINSTALLED ${kdirwatchtest_gui_SRCS})

target_link_libraries(kdirwatchtest_gui ${KDE4_KIO_LIBS})


endif(KDE4_BUILD_TESTS)

########### install files ###############





