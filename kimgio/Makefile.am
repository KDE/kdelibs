

lib_LTLIBRARIES= libkimgio.la kimg_eps.la kimg_xview.la kimg_krl.la \
	$(JPEG_MODULE) $(TIFF_MODULE)

if HAVE_LIBJPEG
JPEG_MODULE = kimg_jpeg.la
endif

if HAVE_LIBTIFF
TIFF_MODULE = kimg_tiff.la
endif

kimg_jpeg_la_SOURCES = jpeg.cpp
kimg_jpeg_la_LDFLAGS = -module -avoid-version $(USER_LDFLAGS)
kimg_jpeg_la_LIBADD = $(LIBJPEG) 

kimg_tiff_la_SOURCES = tiffr.cpp
kimg_tiff_la_LDFLAGS = -module -avoid-version $(USER_LDFLAGS)
kimg_tiff_la_LIBADD = $(LIBTIFF) 

kimg_xview_la_SOURCES = xview.cpp
kimg_xview_la_LDFLAGS = -module -avoid-version $(USER_LDFLAGS)
kimg_xview_la_LIBADD = 

kimg_krl_la_SOURCES = krl.cpp
kimg_krl_la_LDFLAGS = -module -avoid-version $(USER_LDFLAGS)
kimg_krl_la_LIBADD = 

kimg_eps_la_SOURCES = eps.cpp
kimg_eps_la_LDFLAGS = -module -avoid-version $(USER_LDFLAGS)
kimg_eps_la_LIBADD = 

kimgio-config.h: staticmodules

staticmodules: $(JPEG_MODULE) $(TIFF_MODULE)
	@rm -f kimgio-config.h staticmodules ;\
	touch kimgio-config.h ;\
	touch staticmodules ;\
	for mod in jpeg tiff; do \
	if test -f kimg_$$mod.la; then \
	  dlname=`grep -E '^dlname=' kimg_$$mod.la | sed -e 's#^dlname=.\(.*\).#\1#'` ;\
	  if test -z "$$dlname"; then \
		echo "#define LINKED_$$mod 1" >> kimgio-config.h ;\
		case $$mod in \
		jpeg) \
		  echo jpeg.lo $(LIBJPEG) >> staticmodules ;;\
		tiff) \
		  echo tiffr.lo $(LIBTIFF) >> staticmodules ;;\
		esac ;\
	  fi \
	fi ;\
	done; \
	echo "const char* kimgio_rpaths[] = {" >> kimgio-config.h ;\
	  for i in $(KDE_RPATH) "-rpath"; do \
	    if test ! $$i = "-rpath"; then \
	      echo "    \"$$i\"," >> kimgio-config.h  ; \
            fi; \
	  done ;\
	echo "      0};" >> kimgio-config.h

CLEANFILES = staticmodules kimgio-config.h

libkimgio_la_LDFLAGS = -version-info 3:0 --export-dynamic $(all_libraries)
libkimgio_la_LIBADD = `cat staticmodules` $(LIB_KIO)
libkimgio_la_DEPENDENCIES = staticmodules 
libkimgio_la_SOURCES = kimgio.cpp
$(srcdir)/kimgio.cpp: kimgio-config.h
include_HEADERS = kimgio.h

noinst_HEADERS= jpeg.h xview.h eps.h tiffr.h krl.h

INCLUDES = -I$(top_srcdir)/kio -I$(top_srcdir)/dcop -I$(top_srcdir)/libltdl $(all_includes)

EXTRA_DIST = netpbm.cpp netpbm.h

check_PROGRAMS = test
test_SOURCES = test.cpp
test_LDFLAGS = $(KDE_RPATH) 
test_LDADD = libkimgio.la 

