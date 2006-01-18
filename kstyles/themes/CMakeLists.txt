kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kstyle/themes FILES qtplatinum.themerc qtcde.themerc qtmotif.themerc qtwindows.themerc  	qtsgi.themerc qtmotifplus.themerc 	highcolor.themerc  	default.themerc b3.themerc light-v2.themerc  	light-v3.themerc keramik.themerc )
install_files( ${KDE4_CONFIG_DIR} FILES kthemestylerc )

kde4_footer()



#original Makefile.am contents follow:

#
#kstylethemesdata_DATA = qtplatinum.themerc qtcde.themerc qtmotif.themerc qtwindows.themerc \
#	qtsgi.themerc qtmotifplus.themerc\
#	highcolor.themerc \
#	default.themerc b3.themerc light-v2.themerc \
#	light-v3.themerc keramik.themerc
#kstylethemesdatadir = $(kde_datadir)/kstyle/themes
#
#EXTRA_DIST = $(kstylethemesdata_DATA)
#
#kde_conf_DATA = kthemestylerc
#
