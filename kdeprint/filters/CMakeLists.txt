
########### install files ###############

install( FILES enscript.desktop enscript.xml psnup.desktop psnup.xml imagetops.desktop imagetops.xml pdfwrite.desktop pdfwrite.xml psbook1.desktop psbook1.xml psbook2.desktop psbook2.xml psbook.desktop psbook.xml psselect.desktop psselect.xml ps2pdf.desktop ps2pdf.xml poster.desktop poster.xml DESTINATION ${DATA_INSTALL_DIR}/kdeprint/filters)

install(PROGRAMS imagetops DESTINATION ${BIN_INSTALL_DIR} )

