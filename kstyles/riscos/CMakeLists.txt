kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kstyle/pixmaps/riscos FILES arrowdown.png    arrowdown_down.png    arrowleft.png    arrowleft_down.png    arrowright.png    arrowright_down.png    arrowup.png    arrowup_down.png    background.png    check.png    check_checked.png    checkmark.png    combo.png    combo_down.png    hscrollback.png    hscrollbar.png    hscrollbar_down.png    hscrollborder.png    htab.png    menu_back.png    radio.png    radio_down.png    raised_border.png    scrollborder.png    slider.png    sunk_border.png    tab_up.png    vscrollback.png    vscrollbar.png    vscrollbar_down.png    vscrollborder.png    vtab.png )
install_files( ${KDE4_DATA_DIR}/kstyle/themes FILES riscos.themerc )

kde4_footer()



#original Makefile.am contents follow:

#pixmap_DATA = \
#  arrowdown.png \
#  arrowdown_down.png \
#  arrowleft.png \
#  arrowleft_down.png \
#  arrowright.png \
#  arrowright_down.png \
#  arrowup.png \
#  arrowup_down.png \
#  background.png \
#  check.png \
#  check_checked.png \
#  checkmark.png \
#  combo.png \
#  combo_down.png \
#  hscrollback.png \
#  hscrollbar.png \
#  hscrollbar_down.png \
#  hscrollborder.png \
#  htab.png \
#  menu_back.png \
#  radio.png \
#  radio_down.png \
#  raised_border.png \
#  scrollborder.png \
#  slider.png \
#  sunk_border.png \
#  tab_up.png \
#  vscrollback.png \
#  vscrollbar.png \
#  vscrollbar_down.png \
#  vscrollborder.png \
#  vtab.png
#
#pixmapdir = $(kde_datadir)/kstyle/pixmaps/riscos
#
#themerc_DATA = riscos.themerc
#themercdir = $(kde_datadir)/kstyle/themes
#
#EXTRA_DIST = $(pixmap_DATA) $(themerc_DATA)
