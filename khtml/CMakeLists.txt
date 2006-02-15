kde4_header()

project(khtml)

find_package(JPEG REQUIRED)
find_package(GIF  REQUIRED)
find_package(PNG  REQUIRED)

if(MSVC)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -EHsc")
else(MSVC)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
endif(MSVC)

# khtml/dom/Makefile.am: khtmldom

set(khtmldom_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/dom/dom_misc.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/html_block.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/html_inline.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/css_rule.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/dom_node.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/html_document.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/html_list.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/css_stylesheet.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/dom_string.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/html_element.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/html_misc.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/css_value.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/dom_text.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/html_object.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/dom_doc.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/dom_xml.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/html_head.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/dom_element.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/html_base.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/html_image.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/html_form.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/dom2_range.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/html_table.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/dom2_traversal.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/dom2_events.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/dom2_views.cpp
)

# khtml/misc/Makefile.am: khtmlmisc

set(khtmlmisc_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/misc/decoder.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/loader.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/guess_ja.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/htmlhashes.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/helper.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/arena.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/stringit.cpp
)

# khtml/html/Makefile.am: khtmlhtml

set(khtmlhtml_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/html/htmlparser.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/htmltokenizer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/dtd.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_headimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_blockimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_elementimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_inlineimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_documentimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_baseimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_imageimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_listimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_miscimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_formimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_objectimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_tableimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/html_canvasimpl.cpp
)

# khtml/ecma/Makefile.am: kjs_html

set(kjs_html_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_binding.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_dom.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_html.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_window.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_navigator.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_proxy.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_css.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_range.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_traversal.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_events.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_views.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_debugwin.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_mozilla.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/xmlhttprequest.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/xmlserializer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/domparser.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_context2d.cpp
)

# khtml/rendering/Makefile.am: khtmlrender

set(khtmlrender_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/rendering/bidi.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/break_lines.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_block.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_inline.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_style.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_object.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_container.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_box.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_flow.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_text.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_arena.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_layer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_image.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_table.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/table_layout.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_replaced.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_form.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_list.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_canvas.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_frames.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_br.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_body.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/font.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_line.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_generated.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/enumerate.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/counter_tree.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_canvasimage.cpp
)

# khtml/java/Makefile.am: kjava

set(kjava_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/java/kjavaapplet.cpp
  ${CMAKE_SOURCE_DIR}/khtml/java/kjavaappletcontext.cpp
  ${CMAKE_SOURCE_DIR}/khtml/java/kjavaappletserver.cpp
  ${CMAKE_SOURCE_DIR}/khtml/java/kjavaappletwidget.cpp
  ${CMAKE_SOURCE_DIR}/khtml/java/kjavaprocess.cpp
  ${CMAKE_SOURCE_DIR}/khtml/java/kjavadownloader.cpp
)

# khtml/css/Makefile.am: khtmlcss

set(khtmlcss_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/css/css_stylesheetimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/css_ruleimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/css_valueimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/css_base.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/cssparser.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/cssstyleselector.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/csshelper.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/parser.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/css_renderstyledeclarationimpl.cpp
)

# khtml/xml/Makefile.am: khtmlxml

set(khtmlxml_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_docimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_nodeimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_textimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_elementimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_stringimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom2_rangeimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom2_traversalimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/xml_tokenizer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_xmlimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom2_eventsimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom2_viewsimpl.cpp
)

# khtml/imload/Makefile.am: khtmlimload

set(khtmlimload_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/imload/imageplane.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/rawimageplane.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/scaledimageplane.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/pixmapplane.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/animprovider.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/imagepainter.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/updater.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/image.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/imagemanager.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/animtimer.cpp
)

# khtml/imload/decoders/Makefile.am: decoders

set(decoders_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/imload/decoders/jpegloader.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/decoders/pngloader.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/decoders/gifloader.cpp
)

add_subdirectory( misc )
add_subdirectory( dom )
add_subdirectory( css )
add_subdirectory( xml )
add_subdirectory( html )
add_subdirectory( rendering )
add_subdirectory( pics )
add_subdirectory( ecma )
add_subdirectory( imload )
add_subdirectory( kmultipart )

if(NOT KDE4_USE_QT_EMB)
	add_subdirectory( java )
endif(NOT KDE4_USE_QT_EMB )

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories( ${CMAKE_SOURCE_DIR}/kjs ${CMAKE_SOURCE_DIR}/kimgio ${CMAKE_SOURCE_DIR}/dcop ${CMAKE_SOURCE_DIR}/libltdl ${CMAKE_SOURCE_DIR}/kio/kssl ${CMAKE_SOURCE_DIR}/kdeprint ${CMAKE_SOURCE_DIR}/interfaces ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor ${CMAKE_SOURCE_DIR}/kwallet/client ${CMAKE_SOURCE_DIR}/kutils ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}) 
include_directories(${CMAKE_BINARY_DIR}/kio/kssl ${CMAKE_SOURCE_DIR}/khtml/rendering ${CMAKE_SOURCE_DIR}/khtml/ecma ${CMAKE_BINARY_DIR}/khtml/ecma ${CMAKE_SOURCE_DIR}/kio/bookmarks)
include_directories( ${CMAKE_SOURCE_DIR}/khtml/imload ${CMAKE_SOURCE_DIR}/khtml/imload/decoders ${CMAKE_CURRENT_SOURCE_DIR}/misc ${CMAKE_CURRENT_SOURCE_DIR}/dom ${CMAKE_CURRENT_SOURCE_DIR}/html )
include_directories(${JPEG_INCLUDE_DIR} ${GIF_INCLUDE_DIR} ${PNG_INCLUDE_DIR} )

set(CREATE_HASH_TABLE ${CMAKE_SOURCE_DIR}/kjs/create_hash_table )

macro(CREATE_LUT _in_FILE _out_FILE)

   add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
      COMMAND ${PERL_EXECUTABLE}
      ARGS ${CREATE_HASH_TABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} > ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE}
   )
   macro_add_file_dependencies(${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE} )

endmacro(CREATE_LUT)

create_lut(ecma/kjs_dom.cpp kjs_dom.lut.h)
create_lut(ecma/kjs_html.cpp kjs_html.lut.h)
create_lut(ecma/kjs_window.cpp kjs_window.lut.h)
create_lut(ecma/kjs_css.cpp kjs_css.lut.h)
create_lut(ecma/kjs_events.cpp kjs_events.lut.h)
create_lut(ecma/kjs_navigator.cpp kjs_navigator.lut.h)
create_lut(ecma/kjs_mozilla.cpp kjs_mozilla.lut.h)
create_lut(ecma/kjs_range.cpp kjs_range.lut.h)
create_lut(ecma/kjs_traversal.cpp kjs_traversal.lut.h)
create_lut(ecma/kjs_views.cpp kjs_views.lut.h)
create_lut(ecma/xmlhttprequest.cpp xmlhttprequest.lut.h)
create_lut(ecma/xmlserializer.cpp xmlserializer.lut.h)  
create_lut(ecma/domparser.cpp domparser.lut.h)
create_lut(ecma/kjs_context2d.cpp kjs_context2d.lut.h)


########### next target ###############

set(khtml_LIB_SRCS
khtmlview.cpp
khtml_part.cpp
khtml_run.cpp
khtml_factory.cpp
khtml_settings.cc
khtml_events.cpp
khtml_ext.cpp
khtml_pagecache.cpp
khtml_iface.cc
khtml_printsettings.cpp
khtmlimage.cpp
${khtmlxml_STAT_SRCS}
${khtmlhtml_STAT_SRCS}
${khtmlrender_STAT_SRCS}
${khtmlcss_STAT_SRCS}
${khtmlmisc_STAT_SRCS}
${kjs_html_STAT_SRCS}
${khtmldom_STAT_SRCS}
${khtmlimload_STAT_SRCS}
${decoders_STAT_SRCS}
${kjava_STAT_SRCS}
)

kde4_automoc(${khtml_LIB_SRCS})

set( khtml_UI3
kjserrordlg.ui3
htmlpageinfo.ui3
)

kde4_add_ui3_files(khtml_LIB_SRCS ${khtml_UI3} )

set( khtml_DCOP_SKEL_SRCS
khtml_iface.h
)

kde4_add_dcop_skels(khtml_LIB_SRCS ${khtml_DCOP_SKEL_SRCS})

kde4_add_library(khtml SHARED ${khtml_LIB_SRCS})

target_link_libraries(khtml  ${QT_AND_KDECORE_LIBS} kparts kdeprint kutils kde3support kwalletclient kjs ${JPEG_LIBRARIES} ${GIF_LIBRARIES} ${PNG_LIBRARIES} )

set_target_properties(khtml PROPERTIES VERSION 4.2.0 SOVERSION 4 )
install_targets(${KDE4_LIB_INSTALL_DIR} khtml )


########### next target ###############

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
"#include <kdelibs_export.h>\n
extern \"C\"  KDE_EXPORT void* init_libkhtml();\n
extern \"C\"  KDE_EXPORT void* init_libkhtmlpart() { return init_libkhtml(); }\n" )

macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp )

set(khtmlpart_PART_SRCS ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp )

kde4_automoc(${khtmlpart_PART_SRCS})

kde4_add_plugin(khtmlpart WITH_PREFIX ${khtmlpart_PART_SRCS})

target_link_libraries(khtmlpart  ${QT_AND_KDECORE_LIBS} khtml )

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/ khtmlpart )
install_targets(${KDE4_PLUGIN_INSTALL_DIR} khtmlpart )


########### next target ###############


file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/imgdummy.cpp
"#include <kdelibs_export.h>\n
extern \"C\"  KDE_EXPORT void* init_khtmlimagefactory();\n
extern \"C\"  KDE_EXPORT void* init_khtmlimagepart() { return init_khtmlimagefactory(); }\n" )

macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/imgdummy.cpp )

set(khtmlimagepart_PART_SRCS ${CMAKE_CURRENT_BINARY_DIR}/imgdummy.cpp )

kde4_automoc(${khtmlimagepart_PART_SRCS})

kde4_add_plugin(khtmlimagepart ${khtmlimagepart_PART_SRCS})

kde4_install_libtool_file(${KDE4_PLUGIN_INSTALL_DIR}/ khtmlimagepart  )

target_link_libraries(khtmlimagepart  ${QT_AND_KDECORE_LIBS} khtml)

install_targets(${KDE4_LIB_INSTALL_DIR}/kde4 khtmlimagepart )


if(KDE4_BUILD_TESTS)

########### next target ###############

set(testkhtml_SRCS
testkhtml.cpp
domtreeview.cpp
)

kde4_automoc(${testkhtml_SRCS})

kde4_add_executable(testkhtml ${testkhtml_SRCS})

target_link_libraries(testkhtml  ${QT_AND_KDECORE_LIBS} khtml )

########### next target ###############

set(testkhtml_static_SRCS
testkhtml.cpp
domtreeview.cpp
)

kde4_automoc(${testkhtml_static_SRCS})

kde4_add_executable(testkhtml_static ${testkhtml_static_SRCS} )

target_link_libraries(testkhtml_static  ${QT_AND_KDECORE_LIBS} khtml )

########### next target ###############

set(testregression_SRCS
test_regression.cpp
test_regression_fontoverload.cpp
)

kde4_automoc(${testregression_SRCS})

## can't find private/qfontengine_p.h
#kde4_add_executable(testregression ${testregression_SRCS})

#target_link_libraries(testregression  ${QT_AND_KDECORE_LIBS} khtml)

endif(KDE4_BUILD_TESTS)

########### install files ###############

install_files( ${KDE4_SERVICES_DIR} FILES khtml.desktop khtmlimage.desktop )
install_files( ${KDE4_DATA_DIR}/khtml FILES khtml.rc khtml_browser.rc khtml_popupmenu.rc )
install_files( /include FILES khtmlview.h khtml_part.h khtml_events.h khtml_settings.h khtmldefaults.h )

kde4_create_doxygen_docs( REFERENCES kdecore kdeui kio kdefx kparts EXCLUDE test*.* html rendering xml misc pics test  )

kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 1997 Martin Jones (mjones@kde.org)
##              (C) 1997 Torben Weis (weis@kde.org)
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Library General Public
##    License as published by the Free Software Foundation; either
##    version 2 of the License, or (at your option) any later version.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Library General Public License for more details.
#
##    You should have received a copy of the GNU Library General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#KDE_CXXFLAGS = $(WOVERLOADED_VIRTUAL)
#
#if DOJAVA
#JAVA_SUBDIR=java
#LIBKJAVA=
#endif
#
#SUBDIRS = misc dom css xml html rendering pics $(JAVA_SUBDIR) ecma imload . kmultipart
#
#CLEANFILES = dummy.cpp
#
#lib_LTLIBRARIES = libkhtml.la
#kde_module_LTLIBRARIES = libkhtmlpart.la khtmlimagepart.la
#
#libkhtml_la_SOURCES = kjserrordlg.ui3 khtmlview.cpp khtml_part.cpp khtml_run.cpp\
#			khtml_factory.cpp khtml_settings.cc khtml_events.cpp \
#			khtml_ext.cpp khtml_pagecache.cpp khtml_iface.skel \
#			khtml_iface.cc khtml_printsettings.cpp htmlpageinfo.ui3 \
#			khtmlimage.cpp
#
#libkhtml_la_METASOURCES = AUTO
#
#include_HEADERS = khtmlview.h khtml_part.h khtml_events.h khtml_settings.h khtmldefaults.h
#
#noinst_HEADERS = design.h testkhtml.h test_regression.h khtml_ext.h \
#		 khtml_pagecache.h khtmlimage.h khtmlpart_p.h khtml_printsettings.h \
#		kjserrordlg.ui.h
#
#if include_VERSION_SCRIPT
#VSCRIPT = -Wl,--version-script=$(srcdir)/libkhtml.map
#endif
#
#libkhtml_la_LDFLAGS = -version-info 6:0:2 -no-undefined $(VSCRIPT) $(all_libraries)
#libkhtml_la_LIBADD = ./xml/libkhtmlxml.la ./html/libkhtmlhtml.la \
#	./rendering/libkhtmlrender.la ./css/libkhtmlcss.la \
#	./misc/libkhtmlmisc.la ecma/libkjs_html.la ./dom/libkhtmldom.la ./imload/libkhtmlimload.la $(LIBKJAVA) \
#	$(LIB_KPARTS) \
#	$(LIBTHAI) \
#	$(top_builddir)/kdeprint/libkdeprint.la  \
#	$(top_builddir)/kutils/libkutils.la \
#	$(top_builddir)/kde3support/libkde3support.la \
#	$(top_builddir)/kwallet/client/libkwalletclient.la
#
## init_libkhtml is in libkhtml.so; init_libkhtmlpart needs to be in the part
#dummy.cpp: $(srcdir)/Makefile.am
#	echo '#include <kdelibs_export.h>' > $@
#	echo 'extern "C" KDE_EXPORT void* init_libkhtml();' >> $@
#	echo 'extern "C" KDE_EXPORT void* init_libkhtmlpart() { return init_libkhtml(); } ' >> $@
#
#libkhtmlpart_la_SOURCES = dummy.cpp
#libkhtmlpart_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#libkhtmlpart_la_LIBADD = libkhtml.la
#
## init_khtmlimagefactory is in libkhtml.so; init_khtmlimagepart needs to be in the part
#imgdummy.cpp: $(srcdir)/Makefile.am
#	echo '#include <kdelibs_export.h>' > $@
#	echo 'extern "C"  KDE_EXPORT void* init_khtmlimagefactory();' >> $@
#	echo 'extern "C"  KDE_EXPORT void* init_khtmlimagepart() { return init_khtmlimagefactory(); } ' >> $@
#
#khtmlimagepart_la_SOURCES = imgdummy.cpp
#khtmlimagepart_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#khtmlimagepart_la_LIBADD = libkhtml.la
#
#INCLUDES = -I$(top_srcdir)/kjs -I$(top_builddir)/kjs -I$(top_srcdir)/kimgio \
#	-I$(srcdir)/java -I$(top_srcdir)/dcop -I$(srcdir)/misc \
#	-I$(srcdir)/dom -I$(srcdir)/xml -I$(srcdir)/html -I$(srcdir)/css \
#	-I$(top_srcdir)/libltdl \
#	-I$(top_srcdir)/kio/kssl -I$(top_builddir)/kio/kssl \
#	-I$(top_srcdir)/kdeprint \
#	-I$(top_srcdir)/interfaces \
#	-I$(top_srcdir)/interfaces/kregexpeditor \
#        -I$(top_srcdir)/kwallet/client \
#	-I$(top_srcdir)/kutils \
#	-I$(top_srcdir) $(all_includes)
#
#servicedir = $(kde_servicesdir)
#service_DATA = khtml.desktop khtmlimage.desktop
#
#rcdir = $(kde_datadir)/khtml
#rc_DATA = khtml.rc khtml_browser.rc khtml_popupmenu.rc
#
#SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/khtml
#
### generate lib documentation
#srcdoc:
#	$(mkinstalldirs) $(SRCDOC_DEST)
#	kdoc -H -d $(SRCDOC_DEST) kdecore \
#	         $(include_HEADERS) -lqt
#
#EXTRA_DIST =  CHANGES DESIGN README.HTMLWidget README.tags
#
#parser:
#	cd $(srcdir) && \
#	perl scripts/makeattrs && \
#	bash scripts/maketags && \
#	bash scripts/makeprop
#
### test program
#check_PROGRAMS 	= testkhtml
#EXTRA_PROGRAMS = testkhtml_static testregression
#testkhtml_SOURCES = testkhtml.cpp domtreeview.cpp
#testkhtml_LDADD = libkhtml.la
#testkhtml_static_SOURCES = testkhtml.cpp domtreeview.cpp
#testkhtml_static_LDADD = libkhtml.la
#testkhtml_static_LDFLAGS = -static
#testregression_SOURCES = test_regression.cpp test_regression_fontoverload.cpp
#testregression_LDADD = libkhtml.la
#
#DOXYGEN_REFERENCES = kdecore kdeui kio kdefx kparts
#DOXYGEN_EXCLUDE =  test*.* html rendering xml misc pics test
#include ../admin/Doxyfile.am
#
#.PHONY: parser
#
