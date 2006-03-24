kde4_header()

set(kab2kabc_SRCS kab2kabc.cpp kabapi.cc kabaddressbook.cc qconfigDB.cc)

kde4_automoc(${kab2kabc_SRCS})

kde4_add_executable(kab2kabc ${kab2kabc_SRCS})

target_link_libraries(kab2kabc ${KDE4_KDECORE_LIBS} kabc)

install_targets(/bin kab2kabc)

kde4_footer()

