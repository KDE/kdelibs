kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kwalletasync_SRCS
kwalletasync.cpp
kwallettest.cpp
)

kde4_automoc(${kwalletasync_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kwalletasync ${kwalletasync_SRCS})

target_link_libraries(kwalletasync  ${QT_AND_KDECORE_LIBS} kwalletclient )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kwalletsync_SRCS
kwalletsync.cpp
)

kde4_automoc(${kwalletsync_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kwalletsync ${kwalletsync_SRCS})

target_link_libraries(kwalletsync  ${QT_AND_KDECORE_LIBS} kwalletclient )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(kwalletboth_SRCS
kwalletboth.cpp
kwallettest.cpp
)

kde4_automoc(${kwalletboth_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(kwalletboth ${kwalletboth_SRCS})

target_link_libraries(kwalletboth  ${QT_AND_KDECORE_LIBS} kwalletclient )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_ASCII_CAST -I$(srcdir)/../client $(all_includes)
#
#METASOURCES = AUTO
#check_PROGRAMS = kwalletasync kwalletsync kwalletboth
#
#kwalletasync_SOURCES = kwalletasync.cpp kwallettest.cpp
#kwalletasync_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kwalletasync_LDADD = ../client/libkwalletclient.la $(LIB_KDECORE)
#
#kwalletsync_SOURCES = kwalletsync.cpp
#kwalletsync_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kwalletsync_LDADD = ../client/libkwalletclient.la $(LIB_KDECORE)
#
#kwalletboth_SOURCES = kwalletboth.cpp kwallettest.cpp
#kwalletboth_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kwalletboth_LDADD = ../client/libkwalletclient.la $(LIB_KDECORE)
