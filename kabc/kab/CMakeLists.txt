kde4_header()

#include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

set(kab2kabc_SRCS kab2kabc.cpp kabapi.cc kabaddressbook.cc qconfigDB.cc)

kde4_automoc(${kab2kabc_SRCS})

kde4_add_executable(kab2kabc ${kab2kabc_SRCS})

target_link_libraries(kab2kabc ${QT_AND_KDECORE_LIBS} kabc)

install_targets(/bin kab2kabc)

kde4_footer()

