project(knewstuff2)

include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(knewstuff2_LIB_SRCS
   ktranslatable.cpp
   author.cpp
   category.cpp
   entry.cpp
   entryhandler.cpp
   entryloader.cpp
   provider.cpp
   providerhandler.cpp
   providerloader.cpp
   engine.cpp
)

kde4_automoc(${knewstuff2_LIB_SRCS})
kde4_add_library(knewstuff2 SHARED ${knewstuff2_LIB_SRCS})

target_link_libraries(knewstuff2 ${KDE4_KDECORE_LIBS} kio)
set_target_properties(knewstuff2 PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS knewstuff2 DESTINATION ${LIB_INSTALL_DIR})
install(FILES author.h category.h engine.h entry.h entryhandler.h entryloader.h ktranslatable.h provider.h providerhandler.h providerloader.h DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff2)

##########################
 
set(knewstuff2_test_SRCS
   knewstuff2_test.cpp
)

kde4_automoc(${knewstuff2_test_SRCS})
kde4_add_executable(knewstuff2_test ${knewstuff2_test_SRCS})

target_link_libraries(knewstuff2_test ${KDE4_KDECORE_LIBS} knewstuff2)

