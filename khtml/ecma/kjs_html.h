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

#include "html/html_documentimpl.h"
#include "html/html_baseimpl.h"
#include "html/html_miscimpl.h"
#include "html/html_formimpl.h"
#include "misc/loader_client.h"

#include "ecma/kjs_binding.h"
#include "ecma/kjs_dom.h"
#include "xml/dom_nodeimpl.h"  // for NodeImpl::Id

namespace KJS {

  class HTMLElement;

  class HTMLDocument : public DOMDocument {
  public:
    HTMLDocument(ExecState *exec, DOM::HTMLDocumentImpl* d);
    ValueImp* getValueProperty(ExecState *exec, int token);
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp* value, int attr = None);
    void putValueProperty(ExecState *exec, int token, ValueImp* value, int /*attr*/);
    
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Title, Referrer, Domain, URL, Body, Location, Cookie,
           Images, Applets, Links, Forms, Layers, Anchors, Scripts, All, Clear, Open, Close,
           Write, WriteLn, GetElementsByName, GetSelection, CaptureEvents, ReleaseEvents,
           BgColor, FgColor, AlinkColor, LinkColor, VlinkColor, LastModified,
           Height, Width, Dir, Frames, CompatMode };
    DOM::HTMLDocumentImpl* impl() const { return static_cast<DOM::HTMLDocumentImpl*>( m_impl.get() ); }
  private:
    static ValueImp *nameGetter(ExecState *exec, const Identifier& name, const PropertySlot& slot);
    static ValueImp *frameNameGetter(ExecState *exec, const Identifier& name, const PropertySlot& slot);
  };

  class HTMLElement : public DOMElement {
  public:
    HTMLElement(ExecState *exec, DOM::HTMLElementImpl* e) : DOMElement(exec, e) { }
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    ValueImp* getValueProperty(ExecState *exec, int token) const;
    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp* value, int attr = None);
    void putValueProperty(ExecState *exec, int token, ValueImp* value, int);
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
           BodyLink, BodyALink, BodyBgColor,  BodyScrollLeft, BodyScrollTop,
           BodyScrollHeight, BodyScrollWidth, BodyOnLoad,
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
           InputChecked, InputForm, InputAccessKey, InputAlign, InputAlt,
           InputName, InputSrc, InputUseMap, InputSelect, InputClick,
           TextAreaAccessKey, TextAreaName, TextAreaDefaultValue, TextAreaSelect,
           TextAreaCols, TextAreaDisabled, TextAreaForm, TextAreaType,
           TextAreaTabIndex, TextAreaReadOnly, TextAreaRows, TextAreaValue,
           TextAreaBlur, TextAreaFocus, ButtonForm, ButtonTabIndex, ButtonName,
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
           AnchorRev, AnchorTabIndex, AnchorTarget, AnchorText, AnchorBlur,
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
           FrameNoResize, IFrameLongDesc, IFrameAlign,
           IFrameFrameBorder, IFrameSrc, IFrameName, IFrameHeight,
           IFrameMarginHeight, IFrameMarginWidth, IFrameScrolling, IFrameWidth,
           IFrameContentDocument, IFrameContentWindow,
           MarqueeStart, MarqueeStop,
           LayerTop, LayerLeft, LayerVisibility, LayerBgColor, LayerClip, LayerDocument, LayerLayers,
           ElementInnerHTML, ElementTitle, ElementId, ElementDir, ElementLang,
           ElementClassName, ElementInnerText, ElementDocument, ElementChildren, ElementAll };

    DOM::HTMLElementImpl* impl() const { return static_cast<DOM::HTMLElementImpl*>(m_impl.get()); }
    ValueImp* indexGetter(ExecState *exec, unsigned index);
  private:
    static ValueImp *formNameGetter(ExecState *exec, const Identifier& name, const PropertySlot& slot);
  
    QString getURLArg(unsigned id) const;
  
    /* Many of properties in the DOM bindings can be implemented by merely returning
      an attribute as the right type, and setting it in similar manner; or perhaps
      returning a collection of appropriate type*/
    enum BoundPropType {
      T_String, //String, to be return by String()
      T_StrOrNl, //String, to be return by getStringOrNull()
      T_Bool,   //Boolean, return true if property is not null
      T_Int,
      T_URL,
      T_TabIdx, //Magic tab-index handling
      T_Res,    //Reserved, ignore sets, return empty string
      T_Coll    //Collection, type is in attrID
    };
      
    struct BoundPropInfo {
      unsigned elId;  //Applicable element type
      int      token; //Token
      BoundPropType type;
      unsigned attrId; //Attribute to get
    };

    ValueImp* handleBoundRead (ExecState* exec, int token) const;
    bool      handleBoundWrite(ExecState* exec, int token, ValueImp* value);

    static const BoundPropInfo bpTable[];

    static QHash<int, const BoundPropInfo*>* s_boundPropInfo;
    static QHash<int, const BoundPropInfo*>* boundPropInfo();
  };


  class HTMLElementFunction : public DOMFunction {
  public:
    HTMLElementFunction(ExecState *exec, int i, int len);
    virtual ValueImp* callAsFunction(ExecState *exec, ObjectImp* thisObj, const List& args);
  private:
    int id;
  };

  class HTMLCollection : public DOMObject {
  public:
    HTMLCollection(ExecState *exec, DOM::HTMLCollectionImpl* c);
    ~HTMLCollection();
    ValueImp* getValueProperty(ExecState *exec, int token);
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);

    virtual ValueImp* callAsFunction(ExecState *exec, ObjectImp* thisObj, const List& args);
    virtual bool implementsCall() const { return true; }
    virtual bool toBoolean(ExecState *) const;
    virtual bool hasProperty(ExecState *exec, const Identifier &p) const;
    enum { Item, NamedItem, Tags };
    ValueImp* getNamedItems(ExecState *exec, const Identifier &propertyName) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    DOM::HTMLCollectionImpl* impl() const { return m_impl.get(); }
    virtual void hide() { hidden = true; }
    ValueImp* indexGetter(ExecState *exec, unsigned index);
  protected:
    SharedPtr<DOM::HTMLCollectionImpl> m_impl;
    bool hidden;
  private:
    static ValueImp *lengthGetter(ExecState *exec, const Identifier& name, const PropertySlot& slot);
    static ValueImp *nameGetter(ExecState *exec, const Identifier& name, const PropertySlot& slot);
  };

  class HTMLSelectCollection : public HTMLCollection {
  public:
    HTMLSelectCollection(ExecState *exec, DOM::HTMLCollectionImpl* c, DOM::HTMLSelectElementImpl* e)
      : HTMLCollection(exec, c), element(e) { }
    virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    virtual void put(ExecState *exec, const Identifier &propertyName, ValueImp* value, int attr = None);
  private:
    SharedPtr<DOM::HTMLSelectElementImpl> element;
    static ValueImp *selectedIndexGetter(ExecState *exec, const Identifier& propertyName, const PropertySlot& slot);
    static ValueImp *selectedValueGetter(ExecState *exec, const Identifier& propertyName, const PropertySlot& slot);
  };

  ////////////////////// Option Object ////////////////////////

  class OptionConstructorImp : public ObjectImp {
  public:
    OptionConstructorImp(ExecState *exec, DOM::DocumentImpl* d);
    virtual bool implementsConstruct() const;
    virtual ObjectImp* construct(ExecState *exec, const List &args);
  private:
    SharedPtr<DOM::DocumentImpl> doc;
  };

  ////////////////////// Image Object ////////////////////////

  class ImageConstructorImp : public ObjectImp {
  public:
    ImageConstructorImp(ExecState *exec, DOM::DocumentImpl* d);
    virtual bool implementsConstruct() const;
    virtual ObjectImp* construct(ExecState *exec, const List &args);
  private:
    SharedPtr<DOM::DocumentImpl> doc;
  };

  ValueImp* getHTMLCollection(ExecState *exec, DOM::HTMLCollectionImpl* c, bool hide=false);
  ValueImp* getSelectHTMLCollection(ExecState *exec, DOM::HTMLCollectionImpl* c, DOM::HTMLSelectElementImpl* e);

  /* Helper function object for determining the number
   * of occurrences of xxxx as in document.xxxx or window.xxxx.
   * The order of the TagLength array is the order of preference.
   */
  class NamedTagLengthDeterminer {
  public:
    struct TagLength {
      DOM::NodeImpl::Id id; unsigned long length; DOM::NodeImpl *last;
    };
    NamedTagLengthDeterminer(const DOM::DOMString& n, TagLength *t, int l)
      : name(n), tags(t), nrTags(l) {}
    void operator () (DOM::NodeImpl *start);
  private:
    const DOM::DOMString& name;
    TagLength *tags;
    int nrTags;
  };


} // namespace

#endif
