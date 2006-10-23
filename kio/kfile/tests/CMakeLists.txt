
if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KDE4_KIO_INCLUDES} )

########### next target ###############

set(kfstest_SRCS
kfstest.cpp
kfdtest.cpp
)

kde4_automoc(${kfstest_SRCS})

kde4_add_executable(kfstest RUN_UNINSTALLED ${kfstest_SRCS})

target_link_libraries(kfstest  ${KDE4_KIO_LIBS} )


########### next target ###############

set(kurlrequestertest_SRCS
kurlrequestertest.cpp
)

kde4_automoc(${kurlrequestertest_SRCS})

kde4_add_executable(kurlrequestertest RUN_UNINSTALLED ${kurlrequestertest_SRCS})

target_link_libraries(kurlrequestertest  ${KDE4_KIO_LIBS} )


########### next target ###############

set(kfiletreeviewtest_SRCS
kfiletreeviewtest.cpp
)

kde4_automoc(${kfiletreeviewtest_SRCS})

kde4_add_executable(kfiletreeviewtest RUN_UNINSTALLED ${kfiletreeviewtest_SRCS})

target_link_libraries(kfiletreeviewtest  ${KDE4_KIO_LIBS} )


########### next target ###############

set(kopenwithtest_SRCS
kopenwithtest.cpp
)

kde4_automoc(${kopenwithtest_SRCS})

kde4_add_executable(kopenwithtest RUN_UNINSTALLED ${kopenwithtest_SRCS})

target_link_libraries(kopenwithtest  ${KDE4_KIO_LIBS} )


########### next target ###############

set(kdirselectdialogtest_SRCS
kdirselectdialogtest.cpp
)

kde4_automoc(${kdirselectdialogtest_SRCS})

kde4_add_executable(kdirselectdialogtest RUN_UNINSTALLED ${kdirselectdialogtest_SRCS})

target_link_libraries(kdirselectdialogtest  ${KDE4_KIO_LIBS} )


########### next target ###############

set(kicondialogtest_SRCS
kicondialogtest.cpp
)

kde4_automoc(${kicondialogtest_SRCS})

kde4_add_executable(kicondialogtest RUN_UNINSTALLED ${kicondialogtest_SRCS})

target_link_libraries(kicondialogtest  ${KDE4_KIO_LIBS} )


########### next target ###############

set(knotifytest_SRCS
knotifytest.cpp
)

kde4_automoc(${knotifytest_SRCS})

kde4_add_executable(knotifytest RUN_UNINSTALLED ${knotifytest_SRCS})

target_link_libraries(knotifytest  ${KDE4_KIO_LIBS} )


########### next target ###############

set(kcustommenueditortest_SRCS
kcustommenueditortest.cpp
)

kde4_automoc(${kcustommenueditortest_SRCS})

kde4_add_executable(kcustommenueditortest RUN_UNINSTALLED ${kcustommenueditortest_SRCS})

target_link_libraries(kcustommenueditortest  ${KDE4_KIO_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############





