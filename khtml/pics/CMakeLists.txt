kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/khtml/icons FILES  )

kde4_install_icons( hicolor )

kde4_footer()



#original Makefile.am contents follow:

#khtmliconsdir = $(kde_datadir)/khtml/icons
#khtmlicons_ICON = bug \
#                  images_display \
#                  wallet_open
#
## Install globally so they show up in when JS debugger is run in Konqueror
#KDE_ICON = dbgnext \
#           dbgrun \
#           dbgrunto \
#           dbgstep
