// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _KJS_HTML_H_
#define _KJS_HTML_H_

#include "dom/html_document.h"
#include "dom/html_base.h"
#include "dom/html_misc.h"
#include "dom/html_form.h"
#include "misc/loader_client.h"

#include "ecma/kjs_binding.h"
#include "ecma/kjs_dom.h"
#include "xml/dom_nodeimpl.h"  // for NodeImpl::Id

namespace KJS {

  class HTMLElement;

  class HTMLDocument : public DOMDocument {
  public:
    HTMLDocument(ExecState *exec, const DOM::HTMLDocument& d);
    virtual Value tryGet(ExecState *exec, const Identifier &propertyName) const;
    virtual void tryPut(ExecState *exec, const Identifier &propertyName, const Value& value, int attr = None);
    void putValueProperty(ExecState *exec, int token, const Value& value, int /*attr*/);
    virtual bool hasProperty(ExecState *exec, const Identifier &propertyName) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Title, Referrer, Domain, URL, Body, Location, Cookie,
           Images, Applets, Links, Forms, Layers, Anchors, Scripts, All, Clear, Open, Close,
           Write, WriteLn, GetElementsByName, GetSelection, CaptureEvents, ReleaseEvents,
           BgColor, FgColor, AlinkColor, LinkColor, VlinkColor, LastModified,
           Height, Width, Dir, Frames, CompatMode };
    DOM::Document toDocument() const { return static_cast<DOM::Document>( node ); }
  };

  DEFINE_PSEUDO_CONSTRUCTOR(HTMLDocumentPseudoCtor)

  class HTMLElement : public DOMElement {
  public:
    HTMLElement(ExecState *exec, const DOM::HTMLElement& e);
    virtual Value tryGet(ExecState *exec, const Identifier &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    virtual void tryPut(ExecState *exec, const Identifier &propertyName, const Value& value, int attr = None);
    void putValueProperty(ExecState *exec, int token, const Value& value, int);
    virtual bool hasProperty(ExecState *exec, const Identifier &propertyName) const;
    virtual UString toString(ExecState *exec) const;
    virtual void pushEventHandlerScope(ExecState *exec, ScopeChain &scope) const;
    virtual const ClassInfo* classInfo() const;
    static const ClassInfo info;

    static const ClassInfo html_info, head_info, link_info, title_info,
      meta_info, base_info, isIndex_info, style_info, body_info, form_info,
      select_info, optGroup_info, option_info, input_info, textArea_info,
      button_info, label_info, fieldSet_info, legend_info, ul_info, ol_info,
      dl_info, dir_info, menu_info, li_info, div_info, p_info, heading_info,
      blockQuote_info, q_info, pre_info, br_info, baseFont_info, font_info,
      hr_info, mod_info, a_info, img_info, object_info, param_info,
      applet_info, map_info, area_info, script_info, table_info,
      caption_info, col_info, tablesection_info, tr_info,
      tablecell_info, frameSet_info, frame_info, iFrame_info, marquee_info, layer_info;

    enum { HtmlVersion, HeadProfile, LinkHref, LinkRel, LinkMedia,
           LinkCharset, LinkDisabled, LinkHrefLang, LinkRev, LinkTarget, LinkType,
           LinkSheet, TitleText, MetaName, MetaHttpEquiv, MetaContent, MetaScheme,
           BaseHref, BaseTarget, IsIndexForm, IsIndexPrompt, StyleDisabled,
           StyleSheet, StyleType, StyleMedia, BodyBackground, BodyVLink, BodyText,
           BodyLink, BodyALink, BodyBgColor,  BodyOnLoad, BodyFocus, 
           FormAction, FormEncType, FormElements, FormLength, FormAcceptCharset,
           FormReset, FormTarget, FormName, FormMethod, FormSubmit, SelectAdd,
           SelectTabIndex, SelectValue, SelectSelectedIndex, SelectLength,
           SelectRemove, SelectForm, SelectBlur, SelectType, SelectOptions,
           SelectDisabled, SelectMultiple, SelectName, SelectSize, SelectFocus,
           OptGroupDisabled, OptGroupLabel, OptionIndex, OptionSelected,
           OptionForm, OptionText, OptionDefaultSelected, OptionDisabled,
           OptionLabel, OptionValue, InputBlur, InputReadOnly, InputAccept,
           InputSize, InputDefaultValue, InputTabIndex, InputValue, InputType,
           InputFocus, InputMaxLength, InputDefaultChecked, InputDisabled,
           InputChecked, InputIndeterminate, InputForm, InputAccessKey, InputAlign, InputAlt,
           InputName, InputSrc, InputUseMap, InputSelect, InputClick,
           InputSelectionStart, InputSelectionEnd, InputSetSelectionRange,
           TextAreaAccessKey, TextAreaName, TextAreaDefaultValue, TextAreaSelect,
           TextAreaCols, TextAreaDisabled, TextAreaForm, TextAreaType,
           TextAreaTabIndex, TextAreaReadOnly, TextAreaRows, TextAreaValue,
           TextAreaBlur, TextAreaFocus, TextAreaSelectionStart, TextAreaSelectionEnd, TextAreaSetSelectionRange,
           TextAreaTextLength, ButtonBlur, ButtonFocus, ButtonForm, ButtonTabIndex, ButtonName,
           ButtonDisabled, ButtonAccessKey, ButtonType, ButtonValue, LabelHtmlFor,
           LabelForm, LabelAccessKey, FieldSetForm, LegendForm, LegendAccessKey,
           LegendAlign, UListType, UListCompact, OListStart, OListCompact,
           OListType, DListCompact, DirectoryCompact, MenuCompact, LIType,
           LIValue, DivAlign, ParagraphAlign, HeadingAlign, BlockQuoteCite,
           QuoteCite, PreWidth, BRClear, BaseFontColor, BaseFontSize,
           BaseFontFace, FontColor, FontSize, FontFace, HRWidth, HRNoShade,
           HRAlign, HRSize, ModCite, ModDateTime, AnchorShape, AnchorRel,
           AnchorAccessKey, AnchorCoords, AnchorHref, AnchorProtocol, AnchorHost,
           AnchorCharset, AnchorHrefLang, AnchorHostname, AnchorType, AnchorFocus,
           AnchorPort, AnchorPathName, AnchorHash, AnchorSearch, AnchorName,
           AnchorRev, AnchorTabIndex, AnchorTarget, AnchorText, AnchorBlur, AnchorClick,
           ImageName, ImageAlign, ImageHspace, ImageVspace, ImageUseMap, ImageAlt,
           ImageLowSrc, ImageWidth, ImageIsMap, ImageBorder, ImageHeight,
           ImageLongDesc, ImageSrc, ImageX, ImageY, ImageComplete, ObjectHspace, ObjectHeight, ObjectAlign,
           ObjectBorder, ObjectCode, ObjectType, ObjectVspace, ObjectArchive,
           ObjectDeclare, ObjectForm, ObjectCodeBase, ObjectCodeType, ObjectData,
           ObjectName, ObjectStandby, ObjectTabIndex, ObjectUseMap, ObjectWidth, ObjectContentDocument,
           ParamName, ParamType, ParamValueType, ParamValue, AppletArchive,
           AppletAlt, AppletCode, AppletWidth, AppletAlign, AppletCodeBase,
           AppletName, AppletHeight, AppletHspace, AppletObject, AppletVspace,
           MapAreas, MapName, AreaHash, AreaHref, AreaTarget, AreaPort, AreaShape,
           AreaCoords, AreaAlt, AreaAccessKey, AreaNoHref, AreaHost, AreaProtocol,
           AreaHostName, AreaPathName, AreaSearch, AreaTabIndex, ScriptEvent,
           ScriptType, ScriptHtmlFor, ScriptText, ScriptSrc, ScriptCharset,
           ScriptDefer, TableSummary, TableTBodies, TableTHead, TableCellPadding,
           TableDeleteCaption, TableCreateCaption, TableCaption, TableWidth,
           TableCreateTFoot, TableAlign, TableTFoot, TableDeleteRow,
           TableCellSpacing, TableRows, TableBgColor, TableBorder, TableFrame,
           TableRules, TableCreateTHead, TableDeleteTHead, TableDeleteTFoot,
           TableInsertRow, TableCaptionAlign, TableColCh, TableColChOff,
           TableColAlign, TableColSpan, TableColVAlign, TableColWidth,
           TableSectionCh, TableSectionDeleteRow, TableSectionChOff,
           TableSectionRows, TableSectionAlign, TableSectionVAlign,
           TableSectionInsertRow, TableRowSectionRowIndex, TableRowRowIndex,
           TableRowChOff, TableRowCells, TableRowVAlign, TableRowCh,
           TableRowAlign, TableRowBgColor, TableRowDeleteCell, TableRowInsertCell,
           TableCellColSpan, TableCellNoWrap, TableCellAbbr, TableCellHeight,
           TableCellWidth, TableCellCellIndex, TableCellChOff, TableCellBgColor,
           TableCellCh, TableCellVAlign, TableCellRowSpan, TableCellHeaders,
           TableCellAlign, TableCellAxis, TableCellScope, FrameSetCols,
           FrameSetRows, FrameSrc, FrameLocation, FrameFrameBorder, FrameScrolling,
           FrameMarginWidth, FrameLongDesc, FrameMarginHeight, FrameName,
           FrameContentDocument, FrameContentWindow,
           FrameNoResize, FrameWidth, FrameHeight, IFrameLongDesc, IFrameAlign,
           IFrameFrameBorder, IFrameSrc, IFrameName, IFrameHeight,
           IFrameMarginHeight, IFrameMarginWidth, IFrameScrolling, IFrameWidth,
           IFrameContentDocument, IFrameContentWindow,
           MarqueeStart, MarqueeStop,
           LayerTop, LayerLeft, LayerVisibility, LayerBgColor, LayerClip, LayerDocument, LayerLayers,
           ElementInnerHTML, ElementTitle, ElementId, ElementDir, ElementLang,
           ElementClassName, ElementInnerText, ElementDocument,
	   ElementChildren, ElementAll, ElementScrollIntoView };

    DOM::HTMLElement toElement() const { return static_cast<DOM::HTMLElement>(node); }
  };

  class HTMLElementFunction : public DOMFunction {
  public:
    HTMLElementFunction(ExecState *exec, int i, int len);
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
  private:
    int id;
  };

  class HTMLCollection : public DOMObject {
  public:
    HTMLCollection(ExecState *exec,  const DOM::HTMLCollection& c);
    HTMLCollection(const KJS::Object& proto, const DOM::HTMLCollection& c);
    ~HTMLCollection();
    virtual Value tryGet(ExecState *exec, const Identifier &propertyName) const;
    virtual Value call(ExecState *exec, Object &thisObj, const List&args);
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    virtual bool implementsCall() const { return true; }
    virtual bool toBoolean(ExecState *) const;
    virtual bool hasProperty(ExecState *exec, const Identifier &p) const;
    virtual ReferenceList propList(ExecState *exec, bool recursive);
    enum { Item, NamedItem, Tags };
    Value getNamedItems(ExecState *exec, const Identifier &propertyName) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    DOM::HTMLCollection toCollection() const { return collection; }
    virtual void hide() { hidden = true; }
  protected:
    DOM::HTMLCollection collection;
    bool hidden;
  };

  class HTMLSelectCollection : public HTMLCollection {
  public:
    enum { Add };
    HTMLSelectCollection(ExecState *exec, const DOM::HTMLCollection& c, const DOM::HTMLSelectElement& e);
    virtual Value tryGet(ExecState *exec, const Identifier &propertyName) const;
    virtual void tryPut(ExecState *exec, const Identifier &propertyName, const Value& value, int attr = None);
    
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    
    DOM::HTMLSelectElement toElement() const { return element; }
  private:
    DOM::HTMLSelectElement element;
  };

  ////////////////////// Option Object ////////////////////////

  class OptionConstructorImp : public ObjectImp {
  public:
    OptionConstructorImp(ExecState *exec, const DOM::Document &d);
    virtual bool implementsConstruct() const;
    virtual Object construct(ExecState *exec, const List &args);
  private:
    DOM::Document doc;
  };

  ////////////////////// Image Object ////////////////////////

  class ImageConstructorImp : public ObjectImp {
  public:
    ImageConstructorImp(ExecState *exec, const DOM::Document &d);
    virtual bool implementsConstruct() const;
    virtual Object construct(ExecState *exec, const List &args);
  private:
    DOM::Document doc;
  };

  Value getHTMLCollection(ExecState *exec, const DOM::HTMLCollection& c, bool hide=false);
  Value getSelectHTMLCollection(ExecState *exec, const DOM::HTMLCollection& c, const DOM::HTMLSelectElement& e);
  
  
  //All the pseudo constructors..
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLHtmlElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLHeadElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLLinkElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLTitleElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLMetaElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLBaseElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLIsIndexElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLStyleElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLBodyElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLFormElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLSelectElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLOptGroupElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLOptionElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLInputElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLTextAreaElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLButtonElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLLabelElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLFieldSetElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLLegendElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLUListElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLOListElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLDListElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLDirectoryElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLMenuElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLLIElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLDivElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLParagraphElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLHeadingElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLBlockQuoteElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLQuoteElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLPreElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLBRElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLBaseFontElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLFontElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLHRElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLModElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLAnchorElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLImageElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLObjectElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLParamElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLAppletElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLMapElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLAreaElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLScriptElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLTableElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLTableCaptionElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLTableColElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLTableSectionElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLTableRowElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLTableCellElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLFrameSetElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLLayerElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLFrameElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLIFrameElementPseudoCtor)
  DEFINE_PSEUDO_CONSTRUCTOR(HTMLMarqueeElementPseudoCtor)
} // namespace

#endif
