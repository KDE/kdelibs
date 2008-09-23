project(sampleformula)
find_package (KDE4 REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/kformula ${QT_INCLUDES} ${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES} ${KDE4_INCLUDES})

set(sampleformula_SRCS
  main.cpp
  MainWindow.cpp
)

kde4_add_ui_files(sampleformula_SRCS SampleFormulaWidget.ui)

kde4_add_executable(sampleformula ${sampleformula_SRCS})
target_link_libraries(sampleformula kformulalib kdeui)

install(TARGETS sampleformula ${INSTALL_TARGETS_DEFAULT_ARGS})
