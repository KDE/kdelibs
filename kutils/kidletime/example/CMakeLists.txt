
set (kidletest_SRCS
main.cpp
KIdleTest.cpp
)

kde4_add_executable(kidletest ${kidletest_SRCS})
target_link_libraries(kidletest ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}  ${QT_QTCORE_LIBRARY} kidle)
