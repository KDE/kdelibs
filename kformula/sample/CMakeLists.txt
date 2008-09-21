project(kformulapp)

include_directories(${CMAKE_SOURCE_DIR}/kformula ${QT_INCLUDES} ${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES} ${KDE4_INCLUDES})

set(kformulapp_SRCS
  main.cpp
  MainWindow.cpp
  FormulaWidget.cpp
)

kde4_add_executable(kformulapp ${kformulapp_SRCS})
target_link_libraries(kformulapp kformulalib kdeui)
