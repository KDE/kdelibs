kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

########### next target ###############

set(dcop_SRCS dcop.cpp )

kde4_automoc(${dcop_SRCS})


# in MSVC both target "dcop" (the executable listed here) and DCOP (the library) are
# considered the same and so it complains, that's why rename the target here and give the
# resulting executable the original name
kde4_add_executable(dcop_executable ${dcop_SRCS})
set_target_properties(dcop_executable PROPERTIES OUTPUT_NAME dcop)

target_link_libraries(dcop_executable  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} DCOP)

install_targets(/bin dcop_executable )


########### next target ###############

set(dcopfind_SRCS dcopfind.cpp )

kde4_automoc(${dcopfind_SRCS})

kde4_add_executable(dcopfind ${dcopfind_SRCS})

target_link_libraries(dcopfind  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} DCOP)

install_targets(/bin dcopfind )


########### next target ###############

set(dcopclient_SRCS dcopclient.c )

kde4_add_executable(dcopclient ${dcopclient_SRCS} )

target_link_libraries(dcopclient  ${QT_QTCORE_LIBRARY} DCOP)

install_targets(/bin dcopclient )


########### next target ###############

set(dcopobject_SRCS
dcopobject.c
)

kde4_automoc(${dcopobject_SRCS})

kde4_add_executable(dcopobject ${dcopobject_SRCS})

target_link_libraries(dcopobject  ${QT_QTCORE_LIBRARY} DCOP )

install_targets(/bin dcopobject )


########### next target ###############

set(dcopref_SRCS
dcopref.c
)

kde4_automoc(${dcopref_SRCS})

kde4_add_executable(dcopref ${dcopref_SRCS})

target_link_libraries(dcopref  ${QT_QTCORE_LIBRARY} DCOP)

install_targets(/bin dcopref )


########### next target ###############

set(dcopstart_SRCS
dcopstart.cpp
)

kde4_automoc(${dcopstart_SRCS})

kde4_add_executable(dcopstart ${dcopstart_SRCS})

target_link_libraries(dcopstart  ${QT_QTCORE_LIBRARY} DCOP)

install_targets(/bin dcopstart )


########### next target ###############

set(dcopquit_SRCS
dcopquit.cpp
)

kde4_automoc(${dcopquit_SRCS})

kde4_add_executable(dcopquit ${dcopquit_SRCS})

target_link_libraries(dcopquit  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} DCOP)

install_targets(/bin dcopquit )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES = $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#AM_LDFLAGS   = $(all_libraries)
#DCOP_LIB = ../libDCOP.la
#
######## Files
#
#bin_PROGRAMS = dcop dcopfind dcopclient dcopobject dcopref dcopstart dcopquit
#
#dcop_SOURCES = dcop.cpp
#### LIB_QTCORE not enough, due to QImage/QColor/ etc. streaming
#dcop_LDADD = $(LIB_QT) $(LIB_QT3SUPPORT) $(DCOP_LIB)
#dcop_LDFLAGS = $(KDE_RPATH)
#
#dcopstart_SOURCES = dcopstart.cpp
#dcopstart_LDADD = $(LIB_QTCORE) $(DCOP_LIB)
#dcopstart_LDFLAGS = $(KDE_RPATH)
#
#dcopquit_SOURCES = dcopquit.cpp
#dcopquit_LDADD = $(LIB_QT) $(LIB_QT3SUPPORT) $(DCOP_LIB)
#dcopquit_LDFLAGS = $(KDE_RPATH)
#
#dcopfind_SOURCES = dcopfind.cpp
#### LIB_QTCORE not enough, due to QImage/QColor/ etc. streaming
#dcopfind_LDADD = $(LIB_QT) $(LIB_QT3SUPPORT) $(DCOP_LIB)
#dcopfind_LDFLAGS = $(KDE_RPATH)
#
#dcopclient_SOURCES = dcopclient.c
#dcopobject_SOURCES = dcopobject.c
#dcopref_SOURCES = dcopref.c
