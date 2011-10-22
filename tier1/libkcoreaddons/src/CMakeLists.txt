set(kcoreaddons_OPTIONAL_SRCS )
set(kcoreaddons_OPTIONAL_LIBS )

if(NOT WIN32)
    set(kcoreaddons_OPTIONAL_SRCS caching/kshareddatacache.cpp)
    set(kcoreaddons_OPTIONAL_LIBS ${CMAKE_THREAD_LIBS_INIT})
else(NOT WIN32)
    set(kcoreaddons_OPTIONAL_SRCS
        caching/kshareddatacache_win.cpp
        io/kdirwatch_win.cpp
    )
endif(NOT WIN32)

if (WIN32)
    set(kcoreaddons_OPTIONAL_SRCS
        ${kcoreaddons_OPTIONAL_SRCS}
        io/klockfile_win.cpp
        text/kmacroexpander_win.cpp
    )
endif (WIN32)

if (UNIX)
    set(kcoreaddons_OPTIONAL_SRCS
        ${kcoreaddons_OPTIONAL_SRCS}
        io/klockfile_unix.cpp
        text/kmacroexpander_unix.cpp
    )
endif (UNIX)

set(libkcoreaddons_SRCS
    io/kdirwatch.cpp
    io/kfilesystemtype_p.cpp
    io/ksavefile.cpp
    io/kbackup.cpp
    io/ktempdir.cpp
    io/kurl.cpp
    jobs/kcompositejob.cpp
    jobs/kjob.cpp
    jobs/kjobtrackerinterface.cpp
    jobs/kjobuidelegate.cpp
    kernel/kaboutdata.cpp
    kernel/kcmdlineargs.cpp
    randomness/krandom.cpp
    randomness/krandomsequence.cpp
    text/guess_ja.cpp
    text/kcharsets.cpp
    text/kcodecs.cpp
    text/kencodingdetector.cpp
    text/kencodingprober.cpp
    text/kmacroexpander.cpp
    text/probers/CharDistribution.cpp
    text/probers/ChineseGroupProber.cpp
    text/probers/JapaneseGroupProber.cpp
    text/probers/JpCntx.cpp
    text/probers/LangBulgarianModel.cpp
    text/probers/LangCyrillicModel.cpp
    text/probers/LangGreekModel.cpp
    text/probers/LangHebrewModel.cpp
    text/probers/LangHungarianModel.cpp
    text/probers/LangThaiModel.cpp
    text/probers/UnicodeGroupProber.cpp
    text/probers/nsBig5Prober.cpp
    text/probers/nsCharSetProber.cpp
    text/probers/nsEUCJPProber.cpp
    text/probers/nsEUCKRProber.cpp
    text/probers/nsEUCTWProber.cpp
    text/probers/nsEscCharsetProber.cpp
    text/probers/nsEscSM.cpp
    text/probers/nsGB2312Prober.cpp
    text/probers/nsHebrewProber.cpp
    text/probers/nsLatin1Prober.cpp
    text/probers/nsMBCSGroupProber.cpp
    text/probers/nsMBCSSM.cpp
    text/probers/nsSBCSGroupProber.cpp
    text/probers/nsSBCharSetProber.cpp
    text/probers/nsSJISProber.cpp
    text/probers/nsUniversalDetector.cpp
    text/kstringhandler.cpp
    ${kcoreaddons_OPTIONAL_SRCS}
)


include_directories(
    ${CMAKE_BINARY_DIR}/kdecore/ # kde_export.h (required by kde_file.h)
    ${CMAKE_SOURCE_DIR}/kdecore/util/ # kde_file.h (need to be ported to qfile/qfileinfo/qdir)
    ${CMAKE_SOURCE_DIR}/kdecore/kernel/ # K_GLOBAL_STATIC
    ${CMAKE_SOURCE_DIR}/libinqt5/src/
    ${CMAKE_BINARY_DIR}/libinqt5/src/
    ${CMAKE_CURRENT_BINARY_DIR}/io/
    ${CMAKE_CURRENT_SOURCE_DIR}/io/
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/
    ${CMAKE_CURRENT_SOURCE_DIR}/randomness/
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/
    ${CMAKE_CURRENT_SOURCE_DIR}/text/
)

add_library(kcoreaddons ${LIBRARY_TYPE} ${libkcoreaddons_SRCS})
generate_export_header(kcoreaddons)

target_link_libraries(kcoreaddons ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${kcoreaddons_OPTIONAL_LIBS} inqt5)
set_target_properties(kcoreaddons PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                       SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kcoreaddons ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    caching/kshareddatacache.h
    io/kdirwatch.h
    io/klockfile.h
    io/ksavefile.h
    io/kbackup.h
    io/ktempdir.h
    io/kurl.h
    jobs/kcompositejob.h
    jobs/kcompositejob_p.h
    jobs/kjob.h
    jobs/kjob_p.h
    jobs/kjobtrackerinterface.h
    jobs/kjobuidelegate.h
    kernel/kaboutdata.h
    kernel/kcmdlineargs.h
    randomness/krandom.h
    randomness/krandomsequence.h
    text/kcharsets.h
    text/kcodecs.h
    text/kencodingdetector.h
    text/kencodingprober.h
    text/kmacroexpander.h
    text/kstringhandler.h
    ${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
