kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${HTML_INSTALL_DIR}/en/common FILES 
   artistic-license.html bottom1.png bottom2.png bsd-license.html doctop1.png doctop2.png 
   fdl-license fdl-license.html gpl-license gpl-license.html kde-common.css kde-default.css 
   kde-web.css kde-localised.css.template lgpl-license lgpl-license.html qpl-license.html shadow.png 
   web-docbottom.png x11-license.html xml.dcl web-doctop.png doctop1a.png doctop1b.png 
   1.png 2.png 3.png 4.png 5.png 6.png 7.png 8.png 9.png 10.png doxygen.css header.html footer.html 
   mainheader.html appheader.html mainfooter.html fdl-notice.html favicon.ico kde_gear_64.png 
   docheadergears.png headerbg.png grad.png kmenu.png kde_logo.png kde_logo_bg.png bottom-left.png 
   bottom-middle.png bottom-right.png top-left.png top-middle.png top-right-konqueror.png )

kde4_footer()



#original Makefile.am contents follow:

## logotp3.png, logotp3-small.png and kde-localised.css.template
## are not included in the language-specific common directory
## They serve only as `reference' versions.
#
#commondir = $(kde_htmldir)/en/common
#
#common_DATA = artistic-license.html bottom1.png bottom2.png bsd-license.html \
#	doctop1.png doctop2.png fdl-license fdl-license.html gpl-license \
#	gpl-license.html kde-common.css kde-default.css kde-web.css \
#	kde-localised.css.template lgpl-license lgpl-license.html \
#	qpl-license.html shadow.png web-docbottom.png \
#	x11-license.html xml.dcl web-doctop.png \
#	doctop1a.png doctop1b.png 1.png 2.png 3.png 4.png 5.png 6.png \
#	7.png 8.png 9.png 10.png \
#	doxygen.css header.html footer.html mainheader.html appheader.html \
#	mainfooter.html fdl-notice.html \
#	favicon.ico kde_gear_64.png \
#	docheadergears.png headerbg.png grad.png kmenu.png \
#	kde_logo.png kde_logo_bg.png bottom-left.png bottom-middle.png bottom-right.png \
#	top-left.png top-middle.png top-right-konqueror.png
#
