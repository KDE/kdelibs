kde4_header()

if(KDE4_BUILD_TESTS) 
		
include_directories( ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_SOURCE_DIR}/kdecore/network )


########### next target ###############
if (UNIX)
set(netif_SRCS
netif.cpp
)

kde4_automoc(${netif_SRCS})

kde4_add_executable(netif ${netif_SRCS})

target_link_libraries(netif  ${KDE4_KDECORE_LIBS} )
endif (UNIX)

########### next target ###############

set(resolution_SRCS
resolution.cpp
)

kde4_automoc(${resolution_SRCS})

kde4_add_executable(resolution ${resolution_SRCS})

target_link_libraries(resolution  ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(socketdevicetest_SRCS
socketdevicetest.cpp
)

kde4_automoc(${socketdevicetest_SRCS})

kde4_add_executable(socketdevicetest ${socketdevicetest_SRCS})

target_link_libraries(socketdevicetest  ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(streamsockettest_SRCS
streamsockettest.cpp
)

kde4_automoc(${streamsockettest_SRCS})

kde4_add_executable(streamsockettest ${streamsockettest_SRCS})

target_link_libraries(streamsockettest  ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(serversockettest_SRCS
serversockettest.cpp
)

kde4_automoc(${serversockettest_SRCS})

kde4_add_executable(serversockettest ${serversockettest_SRCS})

target_link_libraries(serversockettest  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for libqt-addon
#
#INCLUDES = -I$(srcdir)/.. $(all_includes)
#
#check_PROGRAMS = netif resolution socketdevicetest streamsockettest serversockettest 
#LDADD = ../../libkdecore.la
#
#netif_SOURCES = netif.cpp
#resolution_SOURCES = resolution.cpp
#socketdevicetest_SOURCES = socketdevicetest.cpp
#streamsockettest_SOURCES = streamsockettest.cpp
#serversockettest_SOURCES = serversockettest.cpp
#
#noinst_HEADERS = resolution.h netif.h
#
#METASOURCES = AUTO
