
########### install files ###############

#define exclude file from local/global
kde4_install_icons( ${DATA_INSTALL_DIR}/khtml/icons crystalsvg )




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
