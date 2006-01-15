
#former libtool convenience libraries:

# libltdl/Makefile.am: ltdlc

set(ltdlc_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/libltdl/ltdl.c
)

# kio/kio/Makefile.am: kiocore

set(kiocore_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kio/kio/kdirwatch.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kfileshare.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/ksambashare.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/knfsshare.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/ktrader.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/ktraderparse.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/ktraderparsetree.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kservicetypefactory.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kservicetype.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kmimetype.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kservicegroup.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kservice.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kservicefactory.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kuserprofile.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kservicegroupfactory.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kurifilter.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kshellcompletion.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kurlcompletion.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kautomount.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/krun.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kfileitem.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kdirlister.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kimageio.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/yacc.c
  ${CMAKE_SOURCE_DIR}/kio/kio/lex.c
  ${CMAKE_SOURCE_DIR}/kio/kio/chmodjob.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kscan.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kar.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/ktar.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kzip.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/previewjob.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/metainfojob.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/davjob.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kdatatool.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/karchive.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kfilefilter.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kfilemetainfo.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kdcopservicestarter.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/dataslave.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/dataprotocol.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kacl.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/authinfo.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kprotocolmanager.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/slave.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/slaveinterface.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/sessiondata.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/scheduler.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/connection.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/job.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/global.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/slaveconfig.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kurlpixmapprovider.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/netaccess.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/paste.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/pastedialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kmimemagic.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/tcpslavebase.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/slavebase.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/passdlg.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/forwardingslavebase.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/progressbase.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/defaultprogress.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/statusbarprogress.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kdirnotify.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kdirnotify_stub.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/observer.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kemailsettings.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kprotocolinfo.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/renamedlg.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/skipdlg.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kremoteencoding.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kmimetypechooser.cpp
)

# kio/httpfilter/Makefile.am: httpfilter

set(httpfilter_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc
)

# kio/bookmarks/Makefile.am: kbookmarks

set(kbookmarks_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmark.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkbar.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkimporter.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkmanager.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkmenu.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkimporter_crash.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkimporter_opera.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkimporter_ie.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkimporter_ns.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkimporter_kde1.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkdombuilder.cc
)

# kio/kfile/Makefile.am: kfile

set(kfile_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilefiltercombo.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfileview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfileiconview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/krecentdocument.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfiledialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kdiroperator.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfiledetailview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kcombiview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kurlrequester.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilepreview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kurlcombobox.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kurlrequesterdlg.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kopenwith.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kpropertiesdialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kicondialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kdirsize.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/krecentdirs.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kdiskfreesp.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kimagefilepreview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilesharedlg.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kurlbar.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kmetaprops.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kpreviewprops.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfiletreeview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfiletreeviewitem.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfiletreebranch.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kdirselectdialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilebookmarkhandler.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilemetainfowidget.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kcustommenueditor.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/knotifydialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilespeedbar.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kpreviewwidgetbase.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilemetapreview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kencodingfiledialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kacleditwidget.cpp
)

# kio/kssl/Makefile.am: kssl

set(kssl_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kio/kssl/kssl.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcertificatehome.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslsettings.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcertificate.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslconnectioninfo.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcertificatefactory.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcertificatecache.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslpeerinfo.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslinfodlg.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcertdlg.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslutils.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/kopenssl.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslpkcs12.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcertchain.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslkeygen.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslx509v3.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslsigners.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslpkcs7.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslpemcallback.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslx509map.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslsession.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksmimecrypto.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcsessioncache.cc
)

# kabc/kab/Makefile.am: kab

set(kab_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kabc/kab/kabapi.cc
  ${CMAKE_SOURCE_DIR}/kabc/kab/addressbook.cc
  ${CMAKE_SOURCE_DIR}/kabc/kab/qconfigDB.cc
)

# kabc/vcardparser/Makefile.am: vcards

set(vcards_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kabc/vcardparser/vcard.cpp
  ${CMAKE_SOURCE_DIR}/kabc/vcardparser/vcardline.cpp
  ${CMAKE_SOURCE_DIR}/kabc/vcardparser/vcardparser.cpp
)

# kutils/ksettings/Makefile.am: ksettings

set(ksettings_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kutils/ksettings/dispatcher.cpp
  ${CMAKE_SOURCE_DIR}/kutils/ksettings/dialog.cpp
  ${CMAKE_SOURCE_DIR}/kutils/ksettings/pluginpage.cpp
  ${CMAKE_SOURCE_DIR}/kutils/ksettings/componentsdialog.cpp
)

# kspell2/Makefile.am: kspell2base

set(kspell2base_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kspell2/settings.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/broker.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/client.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/filter.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/backgroundchecker.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/backgroundthread.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/backgroundengine.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/defaultdictionary.cpp
)

# kspell2/ui/Makefile.am: kspell2_noinst

set(kspell2_noinst_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kspell2/ui/configwidget.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/ui/highlighter.cpp
  ${CMAKE_SOURCE_DIR}/kspell2/ui/configdialog.cpp
)

# kdecore/svgicons/Makefile.am: kdesvgicons

set(kdesvgicons_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kdecore/svgicons/ksvgiconengine.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/svgicons/ksvgiconpainter.cpp
)

# kdecore/malloc/Makefile.am: klmalloc

set(klmalloc_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kdecore/malloc/malloc.c
)

# kdecore/network/Makefile.am: kdecorenetwork

set(kdecorenetwork_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kdecore/network/kresolver.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/kresolvermanager.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/kresolverworkerbase.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/ksocketaddress.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/kresolverstandardworkers.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/kreverseresolver.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/ksocketdevice.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/ksocketbase.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/kclientsocketbase.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/kstreamsocket.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/kserversocket.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/kdatagramsocket.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/khttpproxysocketdevice.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/ksockssocketdevice.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/kbufferedsocket.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/ksocketbuffer.cpp
  ${CMAKE_SOURCE_DIR}/kdecore/network/knetworkinterface.cpp
)

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

# kde3support/kdeui/Makefile.am: kde3ui

set(kde3ui_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3dockwidget.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3dockwidget_private.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3popupmenu.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3textedit.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdeui/k3colordrag.cpp
)

# kde3support/kio/Makefile.am: k3io

set(k3io_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kde3support/kio/k3bookmarkdrag.cpp
)

# kde3support/kmdi/Makefile.am: k3mdi

set(k3mdi_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdichildarea.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdichildfrm.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdichildfrmcaption.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdichildview.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdimainfrm.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mditaskbar.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdidockcontainer.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mditoolviewaccessor.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdiguiclient.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdidocumentviewtabwidget.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kmdi/k3mdifocuslist.cpp
)

# kde3support/kparts/Makefile.am: k3parts

set(k3parts_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kde3support/kparts/dockmainwindow3.cpp
)

# kde3support/kdecore/Makefile.am: kde3core

set(kde3core_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kde3support/kdecore/k3shortcut.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdecore/k3keysequence.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdecore/k3shortcutmenu.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdecore/k3urldrag.cpp
  ${CMAKE_SOURCE_DIR}/kde3support/kdecore/k3multipledrag.cpp
)

# dcop/KDE-ICE/Makefile.am: kICE

set(kICE_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/accept.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/authutil.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/connect.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/error.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/getauth.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/iceauth.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/listen.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/listenwk.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/locking.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/misc.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/ping.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/process.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/protosetup.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/register.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/replywait.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/setauth.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/shutdown.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/watch.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/transport.c
  ${CMAKE_SOURCE_DIR}/dcop/KDE-ICE/globals.c
)

# kparts/tests/Makefile.am: notepad

set(notepad_STAT_SRCS 
  ${CMAKE_SOURCE_DIR}/kparts/tests/notepad.cpp
)

