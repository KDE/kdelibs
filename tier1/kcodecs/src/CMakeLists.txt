
if (WIN32)
    list(APPEND kcodecs_OPTIONAL_SRCS
        kmacroexpander_win.cpp
    )
endif (WIN32)


if (UNIX)
    list(APPEND kcodecs_OPTIONAL_SRCS
        kmacroexpander_unix.cpp
    )
endif (UNIX)

set(kcodecs_SRCS
    guess_ja.cpp
    kcharsets.cpp
    kcodecs.cpp
    kencodingdetector.cpp
    kencodingprober.cpp
    kmacroexpander.cpp
    probers/CharDistribution.cpp
    probers/ChineseGroupProber.cpp
    probers/JapaneseGroupProber.cpp
    probers/JpCntx.cpp
    probers/LangBulgarianModel.cpp
    probers/LangCyrillicModel.cpp
    probers/LangGreekModel.cpp
    probers/LangHebrewModel.cpp
    probers/LangHungarianModel.cpp
    probers/LangThaiModel.cpp
    probers/UnicodeGroupProber.cpp
    probers/nsBig5Prober.cpp
    probers/nsCharSetProber.cpp
    probers/nsEUCJPProber.cpp
    probers/nsEUCKRProber.cpp
    probers/nsEUCTWProber.cpp
    probers/nsEscCharsetProber.cpp
    probers/nsEscSM.cpp
    probers/nsGB2312Prober.cpp
    probers/nsHebrewProber.cpp
    probers/nsLatin1Prober.cpp
    probers/nsMBCSGroupProber.cpp
    probers/nsMBCSSM.cpp
    probers/nsSBCSGroupProber.cpp
    probers/nsSBCharSetProber.cpp
    probers/nsSJISProber.cpp
    probers/nsUniversalDetector.cpp
    kstringhandler.cpp
)

add_library(kcodecs ${LIBRARY_TYPE} ${kcodecs_SRCS} ${kcodecs_OPTIONAL_SRCS})
generate_export_header(kcodecs)

target_link_libraries(kcodecs
  LINK_PUBLIC ${QT_QTCORE_LIBRARY}
)

set_target_properties(kcodecs PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                         SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kcodecs EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    kcharsets.h
    kcodecs.h
    kencodingdetector.h
    kencodingprober.h
    kmacroexpander.h
    kstringhandler.h
    ${CMAKE_CURRENT_BINARY_DIR}/kcodecs_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
