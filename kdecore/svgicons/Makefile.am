INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libkdesvgicons.la

CPPFLAGS = -DLIBART_COMPILATION

libkdesvgicons_la_SOURCES = ksvgiconengine.cpp ksvgiconpainter.cpp \
							art_affine.c art_alphagamma.c art_bpath.c art_misc.c art_rect.c art_render.c art_render_gradient.c art_render_svp.c art_rgb.c art_svp.c art_svp_intersect.c art_svp_ops.c art_svp_render_aa.c art_svp_vpath.c art_svp_vpath_stroke.c art_svp_wind.c art_vpath.c art_vpath_bpath.c art_vpath_svp.c art_rgba.c

bin_PROGRAMS = ksvgtopng
ksvgtopng_SOURCES = ksvgtopng.cpp
ksvgtopng_LDADD = libkdesvgicons.la $(LIB_QT) $(all_libraries)

