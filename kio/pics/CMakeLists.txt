kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kio_uiserver/icons FILES  )

kde4_footer()



#original Makefile.am contents follow:

#kiouiservericonsdir = $(kde_datadir)/kio_uiserver/icons
#kiouiservericons_ICON = AUTO
#
