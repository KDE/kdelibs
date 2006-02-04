kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/khtml/icons FILES  )

#define exclude file from local/global
kde4_install_icons( crystalsvg /share/apps/khtml )

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
