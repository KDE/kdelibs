
project(khtml)
KDE4_NO_ENABLE_FINAL(khtml)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=6000)

########svg#########
add_definitions(-DENABLE_SVG)
add_definitions(-DENABLE_SVG_FONTS)
add_definitions(-DWTF_PLATFORM_QT)
########svg#########

add_subdirectory( tests )

find_package(JPEG)
macro_log_feature(JPEG_FOUND "libjpeg" "A library for reading and writing JPEG image files" "http://www.ijg.org" TRUE "" "Required by khtml.")

find_package(GIF)
macro_log_feature(GIF_FOUND "giflib" "A library for manipulating GIF format image files" "http://sourceforge.net/projects/giflib" TRUE "" "Required by khtml.")

find_package(PNG)
macro_log_feature(PNG_FOUND "libpng" "A library for manipulating PNG image format files" "http://www.libpng.org/pub/png" TRUE "" "Required by khtml.")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

configure_file(config-khtml.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-khtml.h )

set(khtmlgraphics_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/AffineTransform.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/FloatPoint.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/FloatRect.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/FloatSize.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/IntRect.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/Path.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/GraphicsTypes.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/PathTraversalState.cpp
  #${CMAKE_SOURCE_DIR}/khtml/platform/graphics/Image.cpp
  #${CMAKE_SOURCE_DIR}/khtml/platform/MIMETypeRegistry.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/qt/AffineTransformQt.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/qt/FloatPointQt.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/qt/FloatRectQt.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/qt/IntPointQt.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/qt/IntRectQt.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/qt/IntSizeQt.cpp
  ${CMAKE_SOURCE_DIR}/khtml/platform/graphics/qt/PathQt.cpp
  #${CMAKE_SOURCE_DIR}/khtml/platform/qt/MIMETypeRegistryQt.cpp
)

set(khtmlsvg_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGDocument.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGSVGElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGStyledLocatableElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGLocatable.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGStyledElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGStylable.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGLength.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGTransformable.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGTransform.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGAngle.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGDocumentExtensions.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGParserUtilities.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGTransformDistance.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGTransformList.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGStringList.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGStyledTransformableElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGTests.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGLangSpace.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGExternalResourcesRequired.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGRectElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPreserveAspectRatio.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGFitToViewBox.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGCircleElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGStyleElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGEllipseElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPointList.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGAnimatedPoints.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPolyElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPolygonElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPolylineElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGAnimatedPathData.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPathSegArc.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPathSegClosePath.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPathSegCurvetoCubic.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPathSegCurvetoCubicSmooth.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPathSegCurvetoQuadratic.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPathSegCurvetoQuadraticSmooth.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPathSegLineto.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPathSegLinetoHorizontal.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPathSegLinetoVertical.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPathSegList.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPathSegMoveto.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGPathElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGURIReference.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGStopElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGGradientElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGLinearGradientElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGRadialGradientElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGDefsElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGClipPathElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGGElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGElementInstance.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGElementInstanceList.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGUseElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGLineElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGTextPathElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGTextContentElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGNumberList.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGTextPositioningElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGLengthList.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGTextElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGAElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGScriptElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGTitleElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGDescElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGNames.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGZoomAndPan.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGViewSpec.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGTSpanElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGGlyphElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGAltGlyphElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGHKernElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGMissingGlyphElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGFontElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/SVGFontData.cpp
#############################################################
## GRAPHICS #################################################
#############################################################
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/SVGResource.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/SVGPaintServer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/qt/SVGPaintServerQt.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/SVGPaintServerSolid.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/qt/SVGPaintServerSolidQt.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/SVGResourceClipper.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/qt/SVGResourceClipperQt.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/SVGPaintServerGradient.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/SVGPaintServerLinearGradient.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/SVGPaintServerRadialGradient.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/qt/SVGPaintServerGradientQt.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/qt/SVGPaintServerLinearGradientQt.cpp
  ${CMAKE_SOURCE_DIR}/khtml/svg/graphics/qt/SVGPaintServerRadialGradientQt.cpp
#############################################################
## RENDERING PART BELOW   ###################################
#############################################################
  ${CMAKE_SOURCE_DIR}/khtml/rendering/RenderSVGRoot.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/RenderPath.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/SVGRenderStyleDefs.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/SVGRenderStyle.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/RenderSVGGradientStop.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/RenderSVGContainer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/SVGRenderSupport.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/RenderSVGHiddenContainer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/RenderSVGTransformableContainer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/SVGInlineFlowBox.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/SVGRootInlineBox.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/RenderSVGTextPath.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/SVGInlineTextBox.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/SVGCharacterLayoutInfo.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/RenderSVGInline.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/RenderSVGText.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/RenderSVGBlock.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/RenderSVGInlineText.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/RenderSVGTSpan.cpp
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
  ${CMAKE_SOURCE_DIR}/khtml/dom/QualifiedName.cpp
  ${CMAKE_SOURCE_DIR}/khtml/dom/dom3_xpath.cpp
)

#set_source_files_properties(${khtmldom_STAT_SRCS} PROPERTIES COMPILE_FLAGS ${KDE4_ENABLE_EXCEPTIONS})

# khtml/misc/Makefile.am: khtmlmisc

set(khtmlmisc_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/misc/loader.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/helper.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/arena.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/stringit.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/paintbuffer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/imagefilter.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/borderarcstroker.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/idstring.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/htmlnames.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/AtomicString.cpp
  ${CMAKE_SOURCE_DIR}/khtml/misc/woff.cpp
)

set(khtmlediting_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/editing/jsediting.cpp
  ${CMAKE_SOURCE_DIR}/khtml/editing/editing.cpp
  ${CMAKE_SOURCE_DIR}/khtml/editing/editor.cpp
  ${CMAKE_SOURCE_DIR}/khtml/editing/htmlediting_impl.cpp
)

set(khtmlfind_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/ui/findbar/khtmlfind.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ui/findbar/khtmlfindbar.cpp
)

kde4_add_ui_files(khtmlfind_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/ui/findbar/khtmlfindbar_base.ui
)

set(passwordbar_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/ui/passwordbar/storepassbar.cpp
)

kde4_add_ui_files(passwordbar_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/ui/passwordbar/storepassbar_base.ui
)

# khtml/html/Makefile.am: khtmlhtml

set(khtmlhtml_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/html/htmlparser.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/htmltokenizer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/htmlprospectivetokenizer.cpp
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
  ${CMAKE_SOURCE_DIR}/khtml/html/HTMLMediaElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/HTMLAudioElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/HTMLVideoElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/HTMLSourceElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/html/TimeRanges.cpp
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
#  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_debugwin.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_mozilla.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/JSTimeRanges.h
  ${CMAKE_CURRENT_BINARY_DIR}/JSTimeRanges.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/JSMediaError.h
  ${CMAKE_CURRENT_BINARY_DIR}/JSMediaError.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/JSHTMLElement.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/JSHTMLMediaElement.h
  ${CMAKE_CURRENT_BINARY_DIR}/JSHTMLMediaElement.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/JSHTMLAudioElement.h
  ${CMAKE_CURRENT_BINARY_DIR}/JSHTMLAudioElement.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/JSHTMLVideoElement.h
  ${CMAKE_CURRENT_BINARY_DIR}/JSHTMLVideoElement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/xmlhttprequest.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/xmlserializer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/domparser.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_context2d.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_audio.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_xpath.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/kjs_scriptable.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/debugger/debugwindow.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/debugger/debugdocument.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/debugger/consoledock.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/debugger/breakpointsdock.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/debugger/callstackdock.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/debugger/localvariabledock.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/debugger/scriptsdock.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/debugger/value2string.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/debugger/errordlg.cpp
  ${CMAKE_SOURCE_DIR}/khtml/ecma/debugger/interpreter_ctx.cpp
)

qt4_add_resources( kjs_html_STAT_SRCS ${CMAKE_SOURCE_DIR}/khtml/ecma/debugger/debugger.qrc )

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
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_position.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/render_media.cpp
  ${CMAKE_SOURCE_DIR}/khtml/rendering/media_controls.cpp
)

# khtml/css/Makefile.am: khtmlcss

set(khtmlcss_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/css/css_stylesheetimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/css_ruleimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/css_valueimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/css_svgvalueimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/css_base.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/cssparser.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/cssstyleselector.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/csshelper.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/parser.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/css_renderstyledeclarationimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/css_mediaquery.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/css_svgcssparser.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/SVGCSSStyleSelector.cpp
  ${CMAKE_SOURCE_DIR}/khtml/css/css_webfont.cpp
)

# khtml/xml/Makefile.am: khtmlxml

set(khtmlxml_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_docimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_nodeimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_nodelistimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_textimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_elementimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_stringimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom2_rangeimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom2_traversalimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/xml_tokenizer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_xmlimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom2_eventsimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom2_viewsimpl.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_restyler.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/ClassNames.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_position.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_positioniterator.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom_selection.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/wa_selectors.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xml/dom3_xpathimpl.cpp
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
  ${CMAKE_SOURCE_DIR}/khtml/imload/canvasimage.cpp
)

# khtml/imload/decoders/Makefile.am: decoders

set(decoders_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/imload/decoders/jpegloader.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/decoders/pngloader.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/decoders/gifloader.cpp
  ${CMAKE_SOURCE_DIR}/khtml/imload/decoders/qimageioloader.cpp
)

# khtml/xpath/
set(xpath_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/khtml/xpath/expression.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xpath/functions.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xpath/parsedstatement.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xpath/path.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xpath/predicate.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xpath/step.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xpath/tokenizer.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xpath/parser.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xpath/util.cpp
  ${CMAKE_SOURCE_DIR}/khtml/xpath/variablereference.cpp
#  ${CMAKE_SOURCE_DIR}/khtml/xpath/XPathNSResolverImpl.cpp
)

add_subdirectory( css )
add_subdirectory( pics )
add_subdirectory( ecma )

include_directories( ${KDE4_PHONON_INCLUDES} ${KDE4_KPARTS_INCLUDES} )
include_directories( ${CMAKE_BINARY_DIR}/kio/kssl )
include_directories( ${KDE4_KJS_INCLUDES} ${CMAKE_SOURCE_DIR}/kimgio ${CMAKE_SOURCE_DIR}/kio/kssl ${CMAKE_SOURCE_DIR}/interfaces ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/misc
  ${CMAKE_CURRENT_SOURCE_DIR}/dom
  ${CMAKE_CURRENT_SOURCE_DIR}/xml
  ${CMAKE_CURRENT_SOURCE_DIR}/html
  ${CMAKE_CURRENT_SOURCE_DIR}/rendering
  ${CMAKE_CURRENT_SOURCE_DIR}/ecma
  ${CMAKE_CURRENT_SOURCE_DIR}/imload
  ${CMAKE_CURRENT_SOURCE_DIR}/imload/decoders
  ${CMAKE_CURRENT_SOURCE_DIR}/java
  ${CMAKE_CURRENT_SOURCE_DIR}/svg
  ${CMAKE_CURRENT_SOURCE_DIR}/svg/graphics
  ${CMAKE_CURRENT_SOURCE_DIR}/platform/graphics
  ${CMAKE_CURRENT_SOURCE_DIR}/platform/text
  ${CMAKE_CURRENT_SOURCE_DIR}/platform
  ${CMAKE_CURRENT_SOURCE_DIR}/compat
  ${CMAKE_CURRENT_SOURCE_DIR}/css
  ${CMAKE_CURRENT_SOURCE_DIR}/xpath
)
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/misc
  ${CMAKE_CURRENT_BINARY_DIR}/dom
  ${CMAKE_CURRENT_BINARY_DIR}/xml
  ${CMAKE_CURRENT_BINARY_DIR}/html
  ${CMAKE_CURRENT_BINARY_DIR}/rendering
  ${CMAKE_CURRENT_BINARY_DIR}/ecma
  ${CMAKE_CURRENT_BINARY_DIR}/imload
  ${CMAKE_CURRENT_BINARY_DIR}/imload/decoders
  ${CMAKE_CURRENT_BINARY_DIR}/java
  ${CMAKE_CURRENT_BINARY_DIR}/svg
  ${CMAKE_CURRENT_BINARY_DIR}/svg/graphics
  ${CMAKE_CURRENT_BINARY_DIR}/platform/graphics
  ${CMAKE_CURRENT_BINARY_DIR}/platform/text
  ${CMAKE_CURRENT_SOURCE_DIR}/platform
  ${CMAKE_CURRENT_BINARY_DIR}/compat
  ${CMAKE_CURRENT_BINARY_DIR}/css
)

include_directories(${JPEG_INCLUDE_DIR} ${GIF_INCLUDE_DIR} ${PNG_INCLUDE_DIR} )

add_subdirectory( kmultipart )
add_subdirectory( java )

add_definitions(-DENABLE_VIDEO)

########### libkhtml ###############

set(khtml_LIB_SRCS
   khtmlview.cpp
   khtmlviewbar.cpp
   khtmlviewbarwidget.cpp
   khtml_part.cpp
   khtml_run.cpp
   khtml_global.cpp
   khtml_settings.cpp
   khtml_filter.cpp
   khtml_events.cpp
   khtml_ext.cpp
   khtml_pagecache.cpp
   khtml_printsettings.cpp
   khtml_iface.cc
   khtmlimage.cpp
   kjserrordlg.cpp
   ${xpath_STAT_SRCS}
   ${khtmlfind_STAT_SRCS}
   ${passwordbar_STAT_SRCS}
   ${khtmlxml_STAT_SRCS}
   ${khtmlhtml_STAT_SRCS}
   ${khtmlrender_STAT_SRCS}
   ${khtmlcss_STAT_SRCS}
   ${khtmlmisc_STAT_SRCS}
   ${khtmlediting_STAT_SRCS}
   ${kjs_html_STAT_SRCS}
   ${khtmldom_STAT_SRCS}
   ${khtmlimload_STAT_SRCS}
   ${decoders_STAT_SRCS}
   ${khtmlgraphics_STAT_SRCS} # for WebCore compatibility
   ${khtmlsvg_STAT_SRCS} # for SVG
   ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KHTMLPart.xml # just so that it gets generated
   )


qt4_generate_dbus_interface(khtml_iface.h org.kde.KHTMLPart.xml)

set(kcookiejar_XML ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KCookieServer.xml)
qt4_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/../kioslave/http/kcookiejar/kcookieserver.h org.kde.KCookieServer.xml)
set_source_files_properties(${kcookiejar_XML} PROPERTIES INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../kioslave/http/kcookiejar/kcookiejar_include.h")
qt4_add_dbus_interface(khtml_LIB_SRCS ${kcookiejar_XML} kcookiejar_interface)

kde4_add_ui_files(khtml_LIB_SRCS kjserrordlgbase.ui htmlpageinfo.ui )

kde4_add_library(khtml SHARED ${khtml_LIB_SRCS})

target_link_libraries(khtml ${KDE4_KPARTS_LIBS} ktexteditor kjs ${KDE4_PHONON_LIBS} ${JPEG_LIBRARIES} ${GIF_LIBRARIES} ${PNG_LIBRARIES} ${X11_LIBRARIES})
target_link_libraries(khtml LINK_INTERFACE_LIBRARIES kparts kjs kio kdeui kdecore ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )

set_target_properties(khtml PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION}
                                       SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
                     )


set(CREATE_HASH_TABLE ${CMAKE_SOURCE_DIR}/kjs/create_hash_table )
macro(CREATE_LUT _in_FILE _out_FILE)

   add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
      COMMAND ${PERL_EXECUTABLE} ${CREATE_HASH_TABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} > ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE}
   )
   if(KDE4_ENABLE_FINAL)
     macro_add_file_dependencies(${CMAKE_CURRENT_BINARY_DIR}/khtml_final_cpp.cpp ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE} )
   else(KDE4_ENABLE_FINAL)
     macro_add_file_dependencies(${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE} )
   endif(KDE4_ENABLE_FINAL)
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
create_lut(ecma/kjs_audio.cpp kjs_audio.lut.h)
create_lut(ecma/kjs_xpath.cpp kjs_xpath.lut.h)

macro(create_js_binding _in_FILE)
   get_filename_component(_in_filename ${_in_FILE} NAME_WE)
   set(_out_h_FILE "${CMAKE_CURRENT_BINARY_DIR}/JS${_in_filename}.h")
   set(_out_cpp_FILE "${CMAKE_CURRENT_BINARY_DIR}/JS${_in_filename}.cpp")
   set(_scripts_dir ${CMAKE_SOURCE_DIR}/khtml/bindings/scripts)
   add_custom_command(OUTPUT ${_out_h_FILE} ${_out_cpp_FILE}
      COMMAND ${PERL_EXECUTABLE} -I${_scripts_dir}
              ${_scripts_dir}/generate-bindings.pl
              --include=${CMAKE_SOURCE_DIR}/khtml/html
              --generator=JS
              --outputdir=${CMAKE_CURRENT_BINARY_DIR}
              --preprocessor=\"${QT_MOC_EXECUTABLE} -E\"
              --defines=ENABLE_VIDEO
              ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE}
      MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE}
      DEPENDS ${_scripts_dir}/CodeGeneratorJS.pm
      )
   if(KDE4_ENABLE_FINAL)
     macro_add_file_dependencies(${CMAKE_CURRENT_BINARY_DIR}/khtml_final_cpp.cpp ${_out_h_FILE} ${_out_cpp_FILE})
   else(KDE4_ENABLE_FINAL)
      macro_add_file_dependencies(${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} ${_out_h_FILE} ${_out_cpp_FILE})
   endif(KDE4_ENABLE_FINAL)
endmacro(create_js_binding)

create_js_binding(html/TimeRanges.idl)
create_js_binding(html/MediaError.idl)
create_js_binding(html/HTMLMediaElement.idl)
create_js_binding(html/HTMLAudioElement.idl)
create_js_binding(html/HTMLVideoElement.idl)
#create_js_binding(svg/SVGRectElement.idl)
#create_js_binding(svg/SVGRect.idl)
#create_js_binding(svg/SVGElement.idl)
#create_js_binding(svg/SVGDocument.idl)
#create_js_binding(svg/SVGAngle.idl)
#create_js_binding(svg/SVGSVGElement.idl)
#create_js_binding(svg/SVGLength.idl)
#create_js_binding(svg/SVGNumber.idl)
#create_js_binding(svg/SVGPoint.idl)
#create_js_binding(svg/SVGMatrix.idl)
#create_js_binding(svg/SVGTransform.idl)
#create_js_binding(svg/SVGException.idl)
#create_js_binding(svg/SVGPaint.idl)
#create_js_binding(svg/SVGNumberList.idl)
#create_js_binding(svg/SVGAnimatedLength.idl)
#create_js_binding(svg/SVGRectElement.idl)
#create_js_binding(svg/SVGTransformList.idl)
#create_js_binding(svg/SVGAnimatedTransformList.idl)
##create_js_binding(svg/SVGLocatable.idl)
##create_js_binding(svg/SVGTransformable.idl)
#create_js_binding(svg/SVGAnimatedString.idl)
#create_js_binding(svg/SVGAnimatedBoolean.idl)
#create_js_binding(svg/SVGStringList.idl)
#create_js_binding(svg/SVGEllipseElement.idl)
#create_js_binding(svg/SVGCircleElement.idl)
#create_js_binding(svg/SVGLineElement.idl)
#create_js_binding(svg/SVGScriptElement.idl)
#create_js_binding(svg/SVGPathSeg.idl)
#create_js_binding(svg/SVGPathElement.idl)
#create_js_binding(svg/SVGAnimatedNumber.idl)
#create_js_binding(svg/SVGPathSegArcAbs.idl)
#create_js_binding(svg/SVGPathSegArcRel.idl)
#create_js_binding(svg/SVGPathSegClosePath.idl)
#create_js_binding(svg/SVGPathSegCurvetoCubicAbs.idl)
#create_js_binding(svg/SVGPathSegCurvetoCubicRel.idl)
#create_js_binding(svg/SVGPathSegCurvetoCubicSmoothAbs.idl)
#create_js_binding(svg/SVGPathSegCurvetoCubicSmoothRel.idl)
#create_js_binding(svg/SVGPathSegCurvetoQuadraticAbs.idl)
#create_js_binding(svg/SVGPathSegCurvetoQuadraticRel.idl)
#create_js_binding(svg/SVGPathSegCurvetoQuadraticSmoothAbs.idl)
#create_js_binding(svg/SVGPathSegCurvetoQuadraticSmoothRel.idl)
#create_js_binding(svg/SVGPathSegLinetoAbs.idl)
#create_js_binding(svg/SVGPathSegLinetoHorizontalAbs.idl)
#create_js_binding(svg/SVGPathSegLinetoHorizontalRel.idl)
#create_js_binding(svg/SVGPathSegLinetoRel.idl)
#create_js_binding(svg/SVGPathSegLinetoVerticalAbs.idl)
#create_js_binding(svg/SVGPathSegLinetoVerticalRel.idl)
#create_js_binding(svg/SVGPathSegList.idl)
#create_js_binding(svg/SVGPathSegMovetoAbs.idl)
#create_js_binding(svg/SVGPathSegMovetoRel.idl)

install(TARGETS khtml EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### khtmlpart ###############

set(khtmlpart_PART_SRCS khtml_factory.cpp )


kde4_add_plugin(khtmlpart WITH_PREFIX ${khtmlpart_PART_SRCS})

target_link_libraries(khtmlpart  ${KDE4_KDECORE_LIBS} khtml )

install(TARGETS khtmlpart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

# Note that khtmlimage.cpp is part of libkhtml because it uses internal objects (render tree and loader)
# Only the entry point is separated into khtmlimage_init.cpp
kde4_add_plugin(khtmlimagepart khtmlimage_init.cpp)


target_link_libraries(khtmlimagepart khtml)

install(TARGETS khtmlimagepart  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############

kde4_add_plugin(khtmladaptorpart khtmladaptorpart.cpp)

target_link_libraries(khtmladaptorpart ${KDE4_KPARTS_LIBS} kjs)

install(TARGETS khtmladaptorpart DESTINATION ${PLUGIN_INSTALL_DIR} )

#######################################

# msvc linker doesn't like "#define protected public"
if(NOT MSVC)

    ########### next target ###############

    set(testkhtml_SRCS
        testkhtml.cpp
    )


    kde4_add_executable(testkhtml ${testkhtml_SRCS})

    target_link_libraries(testkhtml  ${KDE4_KDECORE_LIBS} khtml )

    ########### next target ###############

    set(testkhtml_static_SRCS
        testkhtml.cpp
    )


    kde4_add_executable(testkhtml_static ${testkhtml_static_SRCS} )

    target_link_libraries(testkhtml_static  ${KDE4_KDECORE_LIBS} khtml )

    ########### next target ###############
endif(NOT MSVC)

# Note that testregression can't be compiled by default, it needs private Qt headers.
# See also BUILDING-TESTREGRESSION
if (KHTML_BUILD_TESTREGRESSION)

    set(testregression_SRCS
        test_regression.cpp
        test_regression_fontoverload.cpp
    )

    include_directories( ${QT_INCLUDE_DIR}/../src/3rdparty/harfbuzz/src )

    kde4_add_executable(testregression ${testregression_SRCS})

    target_link_libraries(testregression  ${KDE4_KDECORE_LIBS} ${X11_X11_LIB} khtml)

    ########### next target ###############

    set(testregressiongui_SRCS
        test_regression_gui_window.cpp
        test_regression_gui_main.cpp
    )

    qt4_add_resources(testregressiongui_SRCS test_regression_gui.qrc)

    kde4_add_ui_files(testregressiongui_SRCS test_regression_gui.ui)


    kde4_add_executable(testregressiongui ${testregressiongui_SRCS})

    target_link_libraries(testregressiongui  ${KDE4_KDECORE_LIBS} khtml)
endif (KHTML_BUILD_TESTREGRESSION)

########### install files ###############
install( FILES khtml_export.h khtmlview.h khtml_part.h khtml_events.h khtml_settings.h khtmldefaults.h
         DESTINATION  ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
install( FILES dom/dom_exception.h dom/html_base.h dom/html_inline.h dom/css_rule.h dom/dom_html.h
               dom/html_block.h dom/html_list.h dom/css_stylesheet.h dom/dom_misc.h dom/html_document.h
               dom/html_misc.h dom/css_value.h dom/dom_node.h dom/html_element.h dom/html_object.h
               dom/dom_core.h dom/dom_string.h dom/html_form.h dom/html_table.h dom/dom_doc.h
               dom/dom_text.h dom/html_head.h dom/dom_element.h dom/dom_xml.h dom/html_image.h
               dom/dom2_range.h dom/dom2_traversal.h dom/dom2_events.h dom/dom2_views.h
         DESTINATION  ${INCLUDE_INSTALL_DIR}/dom  COMPONENT Devel)

install( FILES khtml.desktop khtmlimage.desktop
         khtmladaptorpart.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES khtml.rc khtml_browser.rc  DESTINATION  ${DATA_INSTALL_DIR}/khtml )

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KHTMLPart.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

install( FILES error.html DESTINATION ${DATA_INSTALL_DIR}/khtml )

install( FILES khtmlrc DESTINATION ${CONFIG_INSTALL_DIR} )
