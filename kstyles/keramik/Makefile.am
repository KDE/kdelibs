
AM_CPPFLAGS = -DQT_PLUGIN

INCLUDES = -I$(top_srcdir)/kdefx -I$(top_builddir)/kstyles/keramik/ $(all_includes)
# qembed's output needs that...
KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
noinst_HEADERS = keramik.h pixmaploader.h keramikimage.h bitmaps.h
kde_style_LTLIBRARIES = keramik.la
keramik_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
keramik_la_LIBADD = ../../kdefx/libkdefx.la
keramik_la_SOURCES = keramik.cpp pixmaploader.cpp
keramik_la_COMPILE_FIRST = keramikrc.h
keramik_la_METASOURCES = AUTO

noinst_PROGRAMS = genembed

genembed_SOURCES = genembed.cpp
genembed_LDADD = ../../kdefx/libkdefx.la
genembed_LDFLAGS = $(all_libraries) $(KDE_RPATH)

pixmaps.keramik: pics/arrow.png pics/checkbox-off.png pics/checkbox-on.png pics/combobox-list-bc.png\
	 pics/combobox-list-bl.png pics/combobox-list-br.png pics/combobox-list-cl.png pics/combobox-list-cr.png\
	 pics/combobox-list-tc.png pics/combobox-list-tl.png pics/combobox-list-tr.png pics/frame-shadow-cl.png\
	 pics/frame-shadow-tc.png pics/frame-shadow-tl.png pics/listview-bc.png pics/listview-bl.png pics/listview-br.png\
	 pics/listview-cc.png pics/listview-cl.png pics/listview-cr.png pics/listview-pressed-bc.png pics/listview-pressed-bl.png\
	 pics/listview-pressed-br.png pics/listview-pressed-cc.png pics/listview-pressed-cl.png pics/listview-pressed-cr.png\
	 pics/listview-pressed-tc.png pics/listview-pressed-tl.png pics/listview-pressed-tr.png pics/listview-tc.png pics/listview-tl.png\
	 pics/listview-tr.png pics/pushbutton-bc.png pics/pushbutton-bl.png pics/pushbutton-br.png pics/pushbutton-cc.png\
	 pics/pushbutton-cl.png pics/pushbutton-cr.png pics/pushbutton-default-bc.png pics/pushbutton-default-bl.png\
	 pics/pushbutton-default-br.png pics/pushbutton-default-cc.png pics/pushbutton-default-cl.png pics/pushbutton-default-cr.png\
	 pics/pushbutton-default-pressed-bc.png pics/pushbutton-default-pressed-bl.png pics/pushbutton-default-pressed-br.png\
	 pics/pushbutton-default-pressed-cc.png pics/pushbutton-default-pressed-cl.png pics/pushbutton-default-pressed-cr.png\
	 pics/pushbutton-default-pressed-tc.png pics/pushbutton-default-pressed-tl.png pics/pushbutton-default-pressed-tr.png\
	 pics/pushbutton-default-tc.png pics/pushbutton-default-tl.png pics/pushbutton-default-tr.png pics/pushbutton-ocr.png\
	 pics/pushbutton-pressed-bc.png pics/pushbutton-pressed-bl.png pics/pushbutton-pressed-br.png pics/pushbutton-pressed-cc.png\
	 pics/pushbutton-pressed-cl.png pics/pushbutton-pressed-cr.png pics/pushbutton-pressed-tc.png pics/pushbutton-pressed-tl.png\
	 pics/pushbutton-pressed-tr.png pics/pushbutton-small-bc.png pics/pushbutton-small-bl.png pics/pushbutton-small-br.png\
	 pics/pushbutton-small-cc.png pics/pushbutton-small-cl.png pics/pushbutton-small-cr.png\
	 pics/pushbutton-small-pressed-bc.png pics/pushbutton-small-pressed-bl.png pics/pushbutton-small-pressed-br.png\
	 pics/pushbutton-small-pressed-cc.png pics/pushbutton-small-pressed-cl.png pics/pushbutton-small-pressed-cr.png\ 
	 pics/pushbutton-small-pressed-tc.png pics/pushbutton-small-pressed-tl.png pics/pushbutton-small-pressed-tr.png\
	 pics/pushbutton-small-tc.png pics/pushbutton-small-tl.png pics/pushbutton-small-tr.png pics/pushbutton-tc.png\
	 pics/pushbutton-tl.png pics/pushbutton-tr.png pics/radiobutton-off.png pics/radiobutton-on.png pics/ripple.png\
	 pics/scrollbar-hbar-arrow1-pressed.png pics/scrollbar-hbar-arrow1.png pics/scrollbar-hbar-arrow2-pressed.png\
	 pics/scrollbar-hbar-arrow2.png pics/scrollbar-hbar-groove1.png pics/scrollbar-hbar-groove2.png\
	 pics/scrollbar-hbar-slider1.png pics/scrollbar-hbar-slider2.png pics/scrollbar-hbar-slider3.png pics/scrollbar-hbar-slider4.png\
	 pics/scrollbar-hbar-small-slider1.png pics/scrollbar-vbar-arrow1-pressed.png pics/scrollbar-vbar-arrow1.png\
	 pics/scrollbar-vbar-arrow2-pressed.png pics/scrollbar-vbar-arrow2.png pics/scrollbar-vbar-groove1.png\
	 pics/scrollbar-vbar-groove2.png pics/scrollbar-vbar-slider1.png pics/scrollbar-vbar-slider2.png\
	 pics/scrollbar-vbar-slider3.png pics/scrollbar-vbar-slider4.png pics/scrollbar-vbar-small-slider1.png pics/slider-hgroove-bc.png\
	 pics/slider-hgroove-bl.png pics/slider-hgroove-br.png pics/slider-hgroove-cc.png pics/slider-hgroove-cl.png pics/slider-hgroove-cr.png\
	 pics/slider-hgroove-tc.png pics/slider-hgroove-tl.png pics/slider-hgroove-tr.png pics/slider-vgroove-bc.png pics/slider-vgroove-bl.png\
	 pics/slider-vgroove-br.png pics/slider-vgroove-cc.png pics/slider-vgroove-cl.png pics/slider-vgroove-cr.png pics/slider-vgroove-tc.png\
	 pics/slider-vgroove-tl.png pics/slider-vgroove-tr.png pics/slider.png pics/spinbox-1.png pics/spinbox-2.png pics/spinbox-3.png\
	 pics/spinbox-arrow-down.png pics/spinbox-arrow-up.png pics/spinbox-pressed-arrow-down.png pics/spinbox-pressed-arrow-up.png\
	 pics/spinbox-pressed-down.png pics/spinbox-pressed-up.png pics/tab-bottom-active-bc.png pics/tab-bottom-active-bl.png pics/tab-bottom-active-br.png pics/tab-bottom-active-cc.png pics/tab-bottom-active-cl.png pics/tab-bottom-active-cr.png pics/tab-bottom-inactive-bc.png pics/tab-bottom-inactive-bl.png pics/tab-bottom-inactive-br.png pics/tab-bottom-inactive-cc.png pics/tab-bottom-inactive-cl.png pics/tab-bottom-inactive-cr.png pics/tab-bottom-inactive-separator.png pics/tab-top-active-cc.png pics/tab-top-active-cl.png pics/tab-top-active-cr.png pics/tab-top-active-tc.png pics/tab-top-active-tl.png pics/tab-top-active-tr.png pics/tab-top-inactive-cc.png pics/tab-top-inactive-cl.png pics/tab-top-inactive-cr.png pics/tab-top-inactive-separator.png pics/tab-top-inactive-tc.png pics/tab-top-inactive-tl.png pics/tab-top-inactive-tr.png pics/toolbar-bc.png pics/toolbar-bl.png pics/toolbar-br.png pics/toolbar-cc.png pics/toolbar-cl.png pics/toolbar-cr.png pics/toolbar-tc.png pics/toolbar-tl.png pics/toolbar-tr.png pics/vslider.png

pixmaps.keramik keramikrc.h: genembed
	pics=`ls $(srcdir)/pics/*.png 2>/dev/null` ;\ 
	./genembed $$pics > pixmaps.keramik

.PHONY: make-embed

