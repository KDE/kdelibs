kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kdeprint/filters FILES enscript.desktop enscript.xml 	       psnup.desktop psnup.xml  	       imagetops.desktop imagetops.xml  	       pdfwrite.desktop pdfwrite.xml  	       psbook1.desktop psbook1.xml  	       psbook2.desktop psbook2.xml  	       psbook.desktop psbook.xml  	       psselect.desktop psselect.xml  	       ps2pdf.desktop ps2pdf.xml  	       poster.desktop poster.xml )

kde4_footer()



#original Makefile.am contents follow:

#filtersdir = $(kde_datadir)/kdeprint/filters
#filters_DATA = enscript.desktop enscript.xml\
#	       psnup.desktop psnup.xml \
#	       imagetops.desktop imagetops.xml \
#	       pdfwrite.desktop pdfwrite.xml \
#	       psbook1.desktop psbook1.xml \
#	       psbook2.desktop psbook2.xml \
#	       psbook.desktop psbook.xml \
#	       psselect.desktop psselect.xml \
#	       ps2pdf.desktop ps2pdf.xml \
#	       poster.desktop poster.xml
#
#bin_SCRIPTS = imagetops
