
data_DATA = qtplatinum.themerc qtcde.themerc qtmotif.themerc qtwindows.themerc \
	qtmotifplus.themerc qtsgi.themerc \
	system.themerc systemalt.themerc highcolor.themerc mega.themerc \
	kstep.themerc marble.themerc \
	default.themerc b3.themerc light-v2.themerc light-v3.themerc
datadir = $(kde_datadir)/kstyle/themes

EXTRA_DIST = $(data_DATA)

kthemestylerc: kthemestylerc.in
	sed s#KDEPATH#$(prefix)# kthemestylerc.in > kthemestylerc

install-data-local: kthemestylerc
	$(INSTALL) kthemestylerc $(kde_confdir)/
