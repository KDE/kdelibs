kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdeui/about FILES bar-bottom-left.png bar-top-right.png bar-bottom-middle.png bottom-left.png 
   box-middle-left.png top-left.png bar-bottom-right.png bottom-middle.png box-middle-right.png top-middle.png bar-middle-left.png    
   bottom-right.png box-top-left.png bar-middle-right.png box-bottom-left.png box-top-middle.png bar-top-left.png 
   box-bottom-middle.png box-top-right.png bar-top-middle.png box-bottom-right.png kde_infopage.css kde_infopage_rtl.css )

kde4_footer()



#original Makefile.am contents follow:

#
#kde_infopage_datadir = $(kde_datadir)/kdeui/about
#kde_infopage_data_DATA = \
#	bar-bottom-left.png    bar-top-right.png      bar-bottom-middle.png  bottom-left.png \
#	box-middle-left.png    top-left.png           bar-bottom-right.png   bottom-middle.png \
#	box-middle-right.png   top-middle.png         bar-middle-left.png    bottom-right.png \
#	box-top-left.png       bar-middle-right.png   box-bottom-left.png    box-top-middle.png \
#	bar-top-left.png       box-bottom-middle.png  box-top-right.png      bar-top-middle.png \
#	box-bottom-right.png   kde_infopage.css       kde_infopage_rtl.css
