kde4_header()

set(kab2kabc_SRCS kab2kabc.cpp kabapi.cc kabaddressbook.cc qconfigDB.cc)

kde4_automoc(${kab2kabc_SRCS})

kde4_add_executable(kab2kabc NOGUI ${kab2kabc_SRCS})

target_link_libraries(kab2kabc ${KDE4_KDECORE_LIBS} kabc)

install(TARGETS kab2kabc DESTINATION bin)

kde4_footer()

