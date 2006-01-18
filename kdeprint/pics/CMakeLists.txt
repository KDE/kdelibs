kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kdeprint/icons FILES  )
install_files( ${KDE4_DATA_DIR}/kdeprint/pics FILES kdeprint_collate.png  		kdeprint_collate_reverse.png  		kdeprint_color.png  		kdeprint_duplex_none.png  		kdeprint_duplex_long.png  		kdeprint_duplex_short.png  		kdeprint_grayscale.png  		kdeprint_landscape.png  		kdeprint_nup1.png  		kdeprint_nup2.png  		kdeprint_nup4.png  		kdeprint_portrait.png  		kdeprint_revlandscape.png  		kdeprint_revportrait.png  		kdeprint_uncollate.png  		kdeprint_uncollate_reverse.png  		kdeprint_nupother.png  		kdeprint_prettyprint.png )

kde4_footer()



#original Makefile.am contents follow:

#icondata_DATA = kdeprint_collate.png \
#		kdeprint_collate_reverse.png \
#		kdeprint_color.png \
#		kdeprint_duplex_none.png \
#		kdeprint_duplex_long.png \
#		kdeprint_duplex_short.png \
#		kdeprint_grayscale.png \
#		kdeprint_landscape.png \
#		kdeprint_nup1.png \
#		kdeprint_nup2.png \
#		kdeprint_nup4.png \
#		kdeprint_portrait.png \
#		kdeprint_revlandscape.png \
#		kdeprint_revportrait.png \
#		kdeprint_uncollate.png \
#		kdeprint_uncollate_reverse.png \
#		kdeprint_nupother.png \
#		kdeprint_prettyprint.png
#icondatadir = $(kde_datadir)/kdeprint/pics
#
#stdicon_ICON = kdeprint_job kdeprint_job_error kdeprint_job_process kdeprint_job_stopped kdeprint_printer kdeprint_printer_class kdeprint_printer_class_stopped kdeprint_printer_defect kdeprint_printer_remote kdeprint_printer_remote_stopped kdeprint_printer_stopped kdeprint_printer_remote_process kdeprint_printer_process kdeprint_printer_class_process kdeprint_computer kdeprint_printer_infos history kdeprint_printer_users filterstop kdeprint_job_completed kdeprint_job_cancelled package_utilities kdeprint_enableprinter kdeprint_stopprinter kdeprint_addprinter kdeprint_addpseudo kdeprint_configmgr kdeprint_configsrv kdeprint_defaulthard kdeprint_defaultsoft kdeprint_report kdeprint_testprinter kdeprint_uploadsmb kdeprint_restartsrv kdeprint_inklevel kdeprint_printstate kdeprint_queuestate
#stdicondir = $(kde_datadir)/kdeprint/icons
