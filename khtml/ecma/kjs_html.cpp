// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "misc/loader.h"
#include "dom/html_block.h"
#include "dom/html_head.h"
#include "dom/html_image.h"
#include "dom/html_inline.h"
#include "dom/html_list.h"
#include "dom/html_table.h"
#include "dom/html_object.h"
#include "dom/dom_exception.h"

// ### HACK
#include "html/html_baseimpl.h"
#include "html/html_documentimpl.h"

#include "khtml_part.h"
#include "khtmlview.h"

#include "ecma/kjs_css.h"
#include "ecma/kjs_html.h"
#include "ecma/kjs_window.h"
#include "ecma/kjs_html.lut.h"

#include "misc/htmltags.h"

#include <kdebug.h>

using namespace KJS;

IMPLEMENT_PROTOFUNC(HTMLDocFunction)

Value KJS::HTMLDocFunction::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  if (!thisObj.inherits(&HTMLDocument::info)) {
    Object err = Error::create(exec,TypeError);
    exec->setException(err);
    return err;
  }
  DOM::HTMLDocument doc = static_cast<KJS::HTMLDocument *>(thisObj.imp())->toDocument();
  String s;
  DOM::HTMLElement element;

  Value v = args[0];

  switch (id) {
  case HTMLDocument::Clear: // even IE doesn't support that one...
    //doc.clear(); // TODO
    return Undefined();
  case HTMLDocument::Open:
    doc.open();
    return Undefined();
  case HTMLDocument::Close:
    // see khtmltests/ecma/tokenizer-script-recursion.html
    doc.close();
    return Undefined();
  case HTMLDocument::Write:
  case HTMLDocument::WriteLn: {
    // DOM only specifies single string argument, but NS & IE allow multiple
    UString str = v.toString(exec);
    for (int i = 1; i < args.size(); i++)
      str += args[i].toString(exec);
    if (id == HTMLDocument::WriteLn)
      str += "\n";
    //kdDebug() << "document.write: " << str.ascii() << endl;
    doc.write(str.string());
    return Undefined();
  }
  case HTMLDocument::GetElementsByName:
    return getDOMNodeList(exec,doc.getElementsByName(v.toString(exec).string()));
  }

  return Undefined();
}

const ClassInfo KJS::HTMLDocument::info =
  { "HTMLDocument", &DOMDocument::info, &HTMLDocumentTable, 0 };
/* Source for HTMLDocumentTable. Use "make hashtables" to regenerate.
@begin HTMLDocumentTable 31
  title			HTMLDocument::Title		DontDelete
  referrer		HTMLDocument::Referrer		DontDelete|ReadOnly
  domain		HTMLDocument::Domain		DontDelete
  URL			HTMLDocument::URL		DontDelete|ReadOnly
  body			HTMLDocument::Body		DontDelete
  location		HTMLDocument::Location		DontDelete
  cookie		HTMLDocument::Cookie		DontDelete
  images		HTMLDocument::Images		DontDelete|ReadOnly
  applets		HTMLDocument::Applets		DontDelete|ReadOnly
  links			HTMLDocument::Links		DontDelete|ReadOnly
  forms			HTMLDocument::Forms		DontDelete|ReadOnly
  anchors		HTMLDocument::Anchors		DontDelete|ReadOnly
  scripts		HTMLDocument::Scripts		DontDelete|ReadOnly
  all			HTMLDocument::All		DontDelete|ReadOnly
  clear			HTMLDocument::Clear		DontDelete|Function 0
  open			HTMLDocument::Open		DontDelete|Function 0
  close			HTMLDocument::Close		DontDelete|Function 0
  write			HTMLDocument::Write		DontDelete|Function 1
  writeln		HTMLDocument::WriteLn		DontDelete|Function 1
  getElementsByName	HTMLDocument::GetElementsByName	DontDelete|Function 1
  bgColor		HTMLDocument::BgColor		DontDelete
  fgColor		HTMLDocument::FgColor		DontDelete
  alinkColor		HTMLDocument::AlinkColor	DontDelete
  linkColor		HTMLDocument::LinkColor		DontDelete
  vlinkColor		HTMLDocument::VlinkColor	DontDelete
  lastModified		HTMLDocument::LastModified	DontDelete|ReadOnly
  height		HTMLDocument::Height		DontDelete|ReadOnly
  width			HTMLDocument::Width		DontDelete|ReadOnly
  dir			HTMLDocument::Dir		DontDelete
#potentially obsolete array properties
# layers
# plugins
# tags
#potentially obsolete properties
# embeds
# ids
@end
*/
bool KJS::HTMLDocument::hasProperty(ExecState *exec, const UString &p) const
{
#ifdef KJS_VERBOSE
  //kdDebug(6070) << "KJS::HTMLDocument::hasProperty " << p.qstring() << endl;
#endif
  if (!static_cast<DOM::HTMLDocument>(node).all().
      namedItem(p.string()).isNull())
    return true;
  return DOMDocument::hasProperty(exec, p);
}

Value KJS::HTMLDocument::tryGet(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "KJS::HTMLDocument::tryGet " << propertyName.qstring() << endl;
#endif
  DOM::HTMLDocument doc = static_cast<DOM::HTMLDocument>(node);
  DOM::HTMLBodyElement body = doc.body();

  KHTMLView *view = static_cast<DOM::DocumentImpl*>(doc.handle())->view();

  // image and form elements with the name p will be looked up first
  DOM::HTMLCollection coll = doc.all();
  DOM::HTMLElement element = coll.namedItem(propertyName.string());
  if (!element.isNull() &&
      (element.elementId() == ID_IMG || element.elementId() == ID_FORM))
  {
    KJS::HTMLCollection htmlcoll(exec,coll);
    return htmlcoll.getNamedItems(exec, propertyName); // Get all the items with the same name
  }

  const HashEntry* entry = Lookup::findEntry(&HTMLDocumentTable, propertyName);
  if (entry) {
    switch (entry->value) {
    case Title:
      return getString(doc.title());
    case Referrer:
      return getString(doc.referrer());
    case Domain:
      return getString(doc.domain());
    case URL:
      return getString(doc.URL());
    case Body:
      return getDOMNode(exec,doc.body());
    case Location:
      Q_ASSERT(view);
      Q_ASSERT(view->part());
      if ( view && view->part() )
      {
        Window* win = Window::retrieveWindow(view->part());
        if (win)
          return Value(win->location());
	else
          return Undefined();
      }
      else
        return Undefined();
    case Cookie:
      return String(doc.cookie());
    case Images:
      return getHTMLCollection(exec,doc.images());
    case Applets:
      return getHTMLCollection(exec,doc.applets());
    case Links:
      return getHTMLCollection(exec,doc.links());
    case Forms:
      return getHTMLCollection(exec,doc.forms());
    case Anchors:
      return getHTMLCollection(exec,doc.anchors());
    case Scripts: // TODO (IE-specific)
    {
      // To be implemented. Meanwhile, return an object with a length property set to 0
      kdWarning() << "KJS::HTMLDocument document.scripts called - not implemented" << endl;
      Object obj( new ObjectImp() );
      obj.put( exec, "length", Number(0) );
      return obj;
    }
    case All:
      // Disable document.all when we try to be Netscape-compatible
      if ( exec->interpreter()->compatMode() == Interpreter::NetscapeCompat )
        return Undefined();
      return getHTMLCollection(exec,doc.all());
    case Clear:
    case Open:
    case Close:
    case Write:
    case WriteLn:
    case GetElementsByName:
      return lookupOrCreateFunction<HTMLDocFunction>( exec, propertyName, this, entry->value, entry->params, entry->attr );
    }
  }
  // Look for overrides
  ValueImp * val = ObjectImp::getDirect(propertyName);
  if (val)
    return Value(val);

  if (entry) {
    switch (entry->value) {
    case BgColor:
      return String(body.bgColor());
    case FgColor:
      return String(body.text());
    case AlinkColor:
      return String(body.aLink());
    case LinkColor:
      return String(body.link());
    case VlinkColor:
      return String(body.vLink());
    case LastModified:
      return String(doc.lastModified());
    case Height:
      return Number(view ? view->visibleWidth() : 0);
    case Width:
      return Number(view ? view->visibleWidth() : 0);
    case Dir:
      return String(body.dir());
    }
  }
  if (DOMDocument::hasProperty(exec, propertyName))
    return DOMDocument::tryGet(exec, propertyName);

  //kdDebug(6070) << "KJS::HTMLDocument::tryGet " << propertyName.qstring() << " not found, returning element" << endl;
  if(!element.isNull())
  {
    KJS::HTMLCollection htmlcoll(exec,coll);
    return htmlcoll.getNamedItems(exec, propertyName); // Get all the items with the same name
  }
  return Undefined();
}

void KJS::HTMLDocument::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "KJS::HTMLDocument::tryPut " << propertyName.qstring() << endl;
#endif
  DOMObjectLookupPut<HTMLDocument, DOMDocument>( exec, propertyName, value, attr, &HTMLDocumentTable, this );
}

void KJS::HTMLDocument::putValue(ExecState *exec, int token, const Value& value, int /*attr*/)
{
  DOM::HTMLDocument doc = static_cast<DOM::HTMLDocument>(node);
  DOM::HTMLBodyElement body = doc.body();

  switch (token) {
  case Title:
    doc.setTitle(value.toString(exec).string());
    break;
  case Body:
    doc.setBody((new DOMNode(exec, KJS::toNode(value)))->toNode());
    break;
  case Domain: { // not part of the DOM
    DOM::HTMLDocumentImpl* docimpl = static_cast<DOM::HTMLDocumentImpl*>(doc.handle());
    if (docimpl)
      docimpl->setDomain(value.toString(exec).string());
    break;
  }
  case Cookie:
    doc.setCookie(value.toString(exec).string());
    break;
  case Location: {
    KHTMLView *view = static_cast<DOM::DocumentImpl *>( doc.handle() )->view();
    Q_ASSERT(view);
    if (view)
    {
      KHTMLPart *part = view->part();
      QString str = value.toString(exec).qstring();
      part->scheduleRedirection(0, str);
    }
    break;
  }
  case BgColor:
    body.setBgColor(value.toString(exec).string());
    break;
  case FgColor:
    body.setText(value.toString(exec).string());
    break;
  case AlinkColor:
    body.setALink(value.toString(exec).string());
    break;
  case LinkColor:
    body.setLink(value.toString(exec).string());
    break;
  case VlinkColor:
    body.setVLink(value.toString(exec).string());
    break;
  case Dir:
    body.setDir(value.toString(exec).string());
    break;
  default:
    kdWarning() << "HTMLDocument::putValue unhandled token " << token << endl;
  }
}

// -------------------------------------------------------------------------

const ClassInfo KJS::HTMLElement::info = { "HTMLElement", &DOMElement::info, &HTMLElementTable, 0 };
const ClassInfo KJS::HTMLElement::html_info = { "HTMLHtmlElement", &KJS::HTMLElement::info, &HTMLHtmlElementTable, 0 };
const ClassInfo KJS::HTMLElement::head_info = { "HTMLHeadElement", &KJS::HTMLElement::info, &HTMLHeadElementTable, 0 };
const ClassInfo KJS::HTMLElement::link_info = { "HTMLLinkElement", &KJS::HTMLElement::info, &HTMLLinkElementTable, 0 };
const ClassInfo KJS::HTMLElement::title_info = { "HTMLTitleElement", &KJS::HTMLElement::info, &HTMLTitleElementTable, 0 };
const ClassInfo KJS::HTMLElement::meta_info = { "HTMLMetaElement", &KJS::HTMLElement::info, &HTMLMetaElementTable, 0 };
const ClassInfo KJS::HTMLElement::base_info = { "HTMLBaseElement", &KJS::HTMLElement::info, &HTMLBaseElementTable, 0 };
const ClassInfo KJS::HTMLElement::isIndex_info = { "HTMLIsIndexElement", &KJS::HTMLElement::info, &HTMLIsIndexElementTable, 0 };
const ClassInfo KJS::HTMLElement::style_info = { "HTMLStyleElement", &KJS::HTMLElement::info, &HTMLStyleElementTable, 0 };
const ClassInfo KJS::HTMLElement::body_info = { "HTMLBodyElement", &KJS::HTMLElement::info, &HTMLBodyElementTable, 0 };
const ClassInfo KJS::HTMLElement::form_info = { "HTMLFormElement", &KJS::HTMLElement::info, &HTMLFormElementTable, 0 };
const ClassInfo KJS::HTMLElement::select_info = { "HTMLSelectElement", &KJS::HTMLElement::info, &HTMLSelectElementTable, 0 };
const ClassInfo KJS::HTMLElement::optGroup_info = { "HTMLOptGroupElement", &KJS::HTMLElement::info, &HTMLOptGroupElementTable, 0 };
const ClassInfo KJS::HTMLElement::option_info = { "HTMLOptionElement", &KJS::HTMLElement::info, &HTMLOptionElementTable, 0 };
const ClassInfo KJS::HTMLElement::input_info = { "HTMLInputElement", &KJS::HTMLElement::info, &HTMLInputElementTable, 0 };
const ClassInfo KJS::HTMLElement::textArea_info = { "HTMLTextAreaElement", &KJS::HTMLElement::info, &HTMLTextAreaElementTable, 0 };
const ClassInfo KJS::HTMLElement::button_info = { "HTMLButtonElement", &KJS::HTMLElement::info, &HTMLButtonElementTable, 0 };
const ClassInfo KJS::HTMLElement::label_info = { "HTMLLabelElement", &KJS::HTMLElement::info, &HTMLLabelElementTable, 0 };
const ClassInfo KJS::HTMLElement::fieldSet_info = { "HTMLFieldSetElement", &KJS::HTMLElement::info, &HTMLFieldSetElementTable, 0 };
const ClassInfo KJS::HTMLElement::legend_info = { "HTMLLegendElement", &KJS::HTMLElement::info, &HTMLLegendElementTable, 0 };
const ClassInfo KJS::HTMLElement::ul_info = { "HTMLUListElement", &KJS::HTMLElement::info, &HTMLUListElementTable, 0 };
const ClassInfo KJS::HTMLElement::ol_info = { "HTMLOListElement", &KJS::HTMLElement::info, &HTMLOListElementTable, 0 };
const ClassInfo KJS::HTMLElement::dl_info = { "HTMLDListElement", &KJS::HTMLElement::info, &HTMLDListElementTable, 0 };
const ClassInfo KJS::HTMLElement::dir_info = { "HTMLDirectoryElement", &KJS::HTMLElement::info, &HTMLDirectoryElementTable, 0 };
const ClassInfo KJS::HTMLElement::menu_info = { "HTMLMenuElement", &KJS::HTMLElement::info, &HTMLMenuElementTable, 0 };
const ClassInfo KJS::HTMLElement::li_info = { "HTMLLIElement", &KJS::HTMLElement::info, &HTMLLIElementTable, 0 };
const ClassInfo KJS::HTMLElement::div_info = { "HTMLDivElement", &KJS::HTMLElement::info, &HTMLDivElementTable, 0 };
const ClassInfo KJS::HTMLElement::p_info = { "HTMLParagraphElement", &KJS::HTMLElement::info, &HTMLParagraphElementTable, 0 };
const ClassInfo KJS::HTMLElement::heading_info = { "HTMLHeadingElement", &KJS::HTMLElement::info, &HTMLHeadingElementTable, 0 };
const ClassInfo KJS::HTMLElement::blockQuote_info = { "HTMLBlockQuoteElement", &KJS::HTMLElement::info, &HTMLBlockQuoteElementTable, 0 };
const ClassInfo KJS::HTMLElement::q_info = { "HTMLQuoteElement", &KJS::HTMLElement::info, &HTMLQuoteElementTable, 0 };
const ClassInfo KJS::HTMLElement::pre_info = { "HTMLPreElement", &KJS::HTMLElement::info, &HTMLPreElementTable, 0 };
const ClassInfo KJS::HTMLElement::br_info = { "HTMLBRElement", &KJS::HTMLElement::info, &HTMLBRElementTable, 0 };
const ClassInfo KJS::HTMLElement::baseFont_info = { "HTMLBaseFontElement", &KJS::HTMLElement::info, &HTMLBaseFontElementTable, 0 };
const ClassInfo KJS::HTMLElement::font_info = { "HTMLFontElement", &KJS::HTMLElement::info, &HTMLFontElementTable, 0 };
const ClassInfo KJS::HTMLElement::hr_info = { "HTMLHRElement", &KJS::HTMLElement::info, &HTMLHRElementTable, 0 };
const ClassInfo KJS::HTMLElement::mod_info = { "HTMLModElement", &KJS::HTMLElement::info, &HTMLModElementTable, 0 };
const ClassInfo KJS::HTMLElement::a_info = { "HTMLAnchorElement", &KJS::HTMLElement::info, &HTMLAnchorElementTable, 0 };
const ClassInfo KJS::HTMLElement::img_info = { "HTMLImageElement", &KJS::HTMLElement::info, &HTMLImageElementTable, 0 };
const ClassInfo KJS::HTMLElement::object_info = { "HTMLObjectElement", &KJS::HTMLElement::info, &HTMLObjectElementTable, 0 };
const ClassInfo KJS::HTMLElement::param_info = { "HTMLParamElement", &KJS::HTMLElement::info, &HTMLParamElementTable, 0 };
const ClassInfo KJS::HTMLElement::applet_info = { "HTMLAppletElement", &KJS::HTMLElement::info, &HTMLAppletElementTable, 0 };
const ClassInfo KJS::HTMLElement::map_info = { "HTMLMapElement", &KJS::HTMLElement::info, &HTMLMapElementTable, 0 };
const ClassInfo KJS::HTMLElement::area_info = { "HTMLAreaElement", &KJS::HTMLElement::info, &HTMLAreaElementTable, 0 };
const ClassInfo KJS::HTMLElement::script_info = { "HTMLScriptElement", &KJS::HTMLElement::info, &HTMLScriptElementTable, 0 };
const ClassInfo KJS::HTMLElement::table_info = { "HTMLTableElement", &KJS::HTMLElement::info, &HTMLTableElementTable, 0 };
const ClassInfo KJS::HTMLElement::caption_info = { "HTMLTableCaptionElement", &KJS::HTMLElement::info, &HTMLTableCaptionElementTable, 0 };
const ClassInfo KJS::HTMLElement::col_info = { "HTMLTableColElement", &KJS::HTMLElement::info, &HTMLTableColElementTable, 0 };
const ClassInfo KJS::HTMLElement::tablesection_info = { "HTMLTableSectionElement", &KJS::HTMLElement::info, &HTMLTableSectionElementTable, 0 };
const ClassInfo KJS::HTMLElement::tr_info = { "HTMLTableRowElement", &KJS::HTMLElement::info, &HTMLTableRowElementTable, 0 };
const ClassInfo KJS::HTMLElement::tablecell_info = { "HTMLTableCellElement", &KJS::HTMLElement::info, &HTMLTableCellElementTable, 0 };
const ClassInfo KJS::HTMLElement::frameSet_info = { "HTMLFrameSetElement", &KJS::HTMLElement::info, &HTMLFrameSetElementTable, 0 };
const ClassInfo KJS::HTMLElement::frame_info = { "HTMLFrameElement", &KJS::HTMLElement::info, &HTMLFrameElementTable, 0 };
const ClassInfo KJS::HTMLElement::iFrame_info = { "HTMLIFrameElement", &KJS::HTMLElement::info, &HTMLIFrameElementTable, 0 };

const ClassInfo* KJS::HTMLElement::classInfo() const
{
  DOM::HTMLElement element = static_cast<DOM::HTMLElement>(node);
  switch (element.elementId()) {
  case ID_HTML:
    return &html_info;
  case ID_HEAD:
    return &head_info;
  case ID_LINK:
    return &link_info;
  case ID_TITLE:
    return &title_info;
  case ID_META:
    return &meta_info;
  case ID_BASE:
    return &base_info;
  case ID_ISINDEX:
    return &isIndex_info;
  case ID_STYLE:
    return &style_info;
  case ID_BODY:
    return &body_info;
  case ID_FORM:
    return &form_info;
  case ID_SELECT:
    return &select_info;
  case ID_OPTGROUP:
    return &optGroup_info;
  case ID_OPTION:
    return &option_info;
  case ID_INPUT:
    return &input_info;
  case ID_TEXTAREA:
    return &textArea_info;
  case ID_BUTTON:
    return &button_info;
  case ID_LABEL:
    return &label_info;
  case ID_FIELDSET:
    return &fieldSet_info;
  case ID_LEGEND:
    return &legend_info;
  case ID_UL:
    return &ul_info;
  case ID_OL:
    return &ol_info;
  case ID_DL:
    return &dl_info;
  case ID_DIR:
    return &dir_info;
  case ID_MENU:
    return &menu_info;
  case ID_LI:
    return &li_info;
  case ID_DIV:
    return &div_info;
  case ID_P:
    return &p_info;
  case ID_H1:
  case ID_H2:
  case ID_H3:
  case ID_H4:
  case ID_H5:
  case ID_H6:
    return &heading_info;
  case ID_BLOCKQUOTE:
    return &blockQuote_info;
  case ID_Q:
    return &q_info;
  case ID_PRE:
    return &pre_info;
  case ID_BR:
    return &br_info;
  case ID_BASEFONT:
    return &baseFont_info;
  case ID_FONT:
    return &font_info;
  case ID_HR:
    return &hr_info;
  case ID_INS:
  case ID_DEL:
    return &mod_info;
  case ID_A:
    return &a_info;
  case ID_IMG:
    return &img_info;
  case ID_OBJECT:
    return &object_info;
  case ID_PARAM:
    return &param_info;
  case ID_APPLET:
    return &applet_info;
  case ID_MAP:
    return &map_info;
  case ID_AREA:
    return &area_info;
  case ID_SCRIPT:
    return &script_info;
  case ID_TABLE:
    return &table_info;
  case ID_CAPTION:
    return &caption_info;
  case ID_COL:
    return &col_info;
  case ID_THEAD:
    return &tablesection_info;
  case ID_TBODY:
    return &tablesection_info;
  case ID_TFOOT:
    return &tablesection_info;
  case ID_TR:
    return &tr_info;
  case ID_TH:
    return &tablecell_info;
  case ID_TD:
    return &tablecell_info;
  case ID_FRAMESET:
    return &frameSet_info;
  case ID_FRAME:
    return &frame_info;
  case ID_IFRAME:
    return &iFrame_info;
  default:
    return &info;
  }
}
/*
@begin HTMLElementTable 8
  id		KJS::HTMLElement::ElementId	DontDelete
  title		KJS::HTMLElement::ElementTitle	DontDelete
  lang		KJS::HTMLElement::ElementLang	DontDelete
  dir		KJS::HTMLElement::ElementDir	DontDelete
### isn't this "class" in the HTML spec?
  className	KJS::HTMLElement::ElementClassName DontDelete
  innerHTML	KJS::HTMLElement::ElementInnerHTML DontDelete
  innerText	KJS::HTMLElement::ElementInnerText DontDelete
  document	KJS::HTMLElement::ElementDocument  DontDelete|ReadOnly
# IE extension
  children	KJS::HTMLElement::ElementChildren  DontDelete|ReadOnly
@end
@begin HTMLHtmlElementTable 1
  version	KJS::HTMLElement::HtmlVersion	DontDelete
@end
@begin HTMLHeadElementTable 1
  profile	KJS::HTMLElement::HeadProfile	DontDelete
@end
@begin HTMLLinkElementTable 11
  disabled	KJS::HTMLElement::LinkDisabled	DontDelete
  charset	KJS::HTMLElement::LinkCharset	DontDelete
  href		KJS::HTMLElement::LinkHref	DontDelete
  hreflang	KJS::HTMLElement::LinkHrefLang	DontDelete
  media		KJS::HTMLElement::LinkMedia	DontDelete
  rel		KJS::HTMLElement::LinkRel      	DontDelete
  rev		KJS::HTMLElement::LinkRev	DontDelete
  target	KJS::HTMLElement::LinkTarget	DontDelete
  type		KJS::HTMLElement::LinkType	DontDelete
  sheet		KJS::HTMLElement::LinkSheet	DontDelete|ReadOnly
@end
@begin HTMLTitleElementTable 1
  text		KJS::HTMLElement::TitleText	DontDelete
@end
@begin HTMLMetaElementTable 4
  content	KJS::HTMLElement::MetaContent	DontDelete
  httpEquiv	KJS::HTMLElement::MetaHttpEquiv	DontDelete
  name		KJS::HTMLElement::MetaName	DontDelete
  scheme	KJS::HTMLElement::MetaScheme	DontDelete
@end
@begin HTMLBaseElementTable 2
  href		KJS::HTMLElement::BaseHref	DontDelete
  target	KJS::HTMLElement::BaseTarget	DontDelete
@end
@begin HTMLIsIndexElementTable 2
  form		KJS::HTMLElement::IsIndexForm	DontDelete|ReadOnly
  prompt	KJS::HTMLElement::IsIndexPrompt	DontDelete
@end
@begin HTMLStyleElementTable 4
  disabled	KJS::HTMLElement::StyleDisabled	DontDelete
  media		KJS::HTMLElement::StyleMedia	DontDelete
  type		KJS::HTMLElement::StyleType	DontDelete
  sheet		KJS::HTMLElement::StyleSheet	DontDelete|ReadOnly
@end
@begin HTMLBodyElementTable 8
  aLink		KJS::HTMLElement::BodyALink	DontDelete
  background	KJS::HTMLElement::BodyBackground	DontDelete
  bgColor	KJS::HTMLElement::BodyBgColor	DontDelete
  link		KJS::HTMLElement::BodyLink	DontDelete
  text		KJS::HTMLElement::BodyText	DontDelete
  vLink		KJS::HTMLElement::BodyVLink	DontDelete
  scrollHeight	KJS::HTMLElement::BodyScrollHeight	DontDelete|ReadOnly
  scrollWidth	KJS::HTMLElement::BodyScrollWidth	DontDelete|ReadOnly
@end
@begin HTMLFormElementTable 11
# Also supported, by name/index
  elements	KJS::HTMLElement::FormElements	DontDelete|ReadOnly
  length	KJS::HTMLElement::FormLength	DontDelete|ReadOnly
  name		KJS::HTMLElement::FormName	DontDelete
  acceptCharset	KJS::HTMLElement::FormAcceptCharset	DontDelete
  action	KJS::HTMLElement::FormAction	DontDelete
  enctype	KJS::HTMLElement::FormEncType	DontDelete
  method	KJS::HTMLElement::FormMethod	DontDelete
  target	KJS::HTMLElement::FormTarget	DontDelete
  submit	KJS::HTMLElement::FormSubmit	DontDelete|Function 0
  reset		KJS::HTMLElement::FormReset	DontDelete|Function 0
@end
@begin HTMLSelectElementTable 11
# Also supported, by index
  type		KJS::HTMLElement::SelectType	DontDelete|ReadOnly
  selectedIndex	KJS::HTMLElement::SelectSelectedIndex	DontDelete
  value		KJS::HTMLElement::SelectValue	DontDelete
  length	KJS::HTMLElement::SelectLength	DontDelete
  form		KJS::HTMLElement::SelectForm	DontDelete|ReadOnly
  options	KJS::HTMLElement::SelectOptions	DontDelete|ReadOnly
  disabled	KJS::HTMLElement::SelectDisabled	DontDelete
  multiple	KJS::HTMLElement::SelectMultiple	DontDelete
  name		KJS::HTMLElement::SelectName	DontDelete
  size		KJS::HTMLElement::SelectSize	DontDelete
  tabIndex	KJS::HTMLElement::SelectTabIndex	DontDelete
  add		KJS::HTMLElement::SelectAdd	DontDelete|Function 2
  remove	KJS::HTMLElement::SelectRemove	DontDelete|Function 1
  blur		KJS::HTMLElement::SelectBlur	DontDelete|Function 0
  focus		KJS::HTMLElement::SelectFocus	DontDelete|Function 0
@end
@begin HTMLOptGroupElementTable 2
  disabled	KJS::HTMLElement::OptGroupDisabled	DontDelete
  label		KJS::HTMLElement::OptGroupLabel		DontDelete
@end
@begin HTMLOptionElementTable 8
  form		KJS::HTMLElement::OptionForm		DontDelete|ReadOnly
  defaultSelected KJS::HTMLElement::OptionDefaultSelected	DontDelete
  text		KJS::HTMLElement::OptionText		DontDelete
  index		KJS::HTMLElement::OptionIndex		DontDelete|ReadOnly
  disabled	KJS::HTMLElement::OptionDisabled	DontDelete
  label		KJS::HTMLElement::OptionLabel		DontDelete
  selected	KJS::HTMLElement::OptionSelected	DontDelete
  value		KJS::HTMLElement::OptionValue		DontDelete
@end
@begin HTMLInputElementTable 23
  defaultValue	KJS::HTMLElement::InputDefaultValue	DontDelete
  defaultChecked KJS::HTMLElement::InputDefaultChecked	DontDelete
  form		KJS::HTMLElement::InputForm		DontDelete|ReadOnly
  accept	KJS::HTMLElement::InputAccept		DontDelete
  accessKey	KJS::HTMLElement::InputAccessKey	DontDelete
  align		KJS::HTMLElement::InputAlign		DontDelete
  alt		KJS::HTMLElement::InputAlt		DontDelete
  checked	KJS::HTMLElement::InputChecked		DontDelete
  disabled	KJS::HTMLElement::InputDisabled		DontDelete
  maxLength	KJS::HTMLElement::InputMaxLength	DontDelete
  name		KJS::HTMLElement::InputName		DontDelete
  readOnly	KJS::HTMLElement::InputReadOnly		DontDelete
  size		KJS::HTMLElement::InputSize		DontDelete
  src		KJS::HTMLElement::InputSrc		DontDelete
  tabIndex	KJS::HTMLElement::InputTabIndex		DontDelete
  type		KJS::HTMLElement::InputType		DontDelete
  useMap	KJS::HTMLElement::InputUseMap		DontDelete
  value		KJS::HTMLElement::InputValue		DontDelete
  blur		KJS::HTMLElement::InputBlur		DontDelete|Function 0
  focus		KJS::HTMLElement::InputFocus		DontDelete|Function 0
  select	KJS::HTMLElement::InputSelect		DontDelete|Function 0
  click		KJS::HTMLElement::InputClick		DontDelete|Function 0
@end
@begin HTMLTextAreaElementTable 13
  defaultValue	KJS::HTMLElement::TextAreaDefaultValue	DontDelete
  form		KJS::HTMLElement::TextAreaForm		DontDelete|ReadOnly
  accessKey	KJS::HTMLElement::TextAreaAccessKey	DontDelete
  cols		KJS::HTMLElement::TextAreaCols		DontDelete
  disabled	KJS::HTMLElement::TextAreaDisabled	DontDelete
  name		KJS::HTMLElement::TextAreaName		DontDelete
  readOnly	KJS::HTMLElement::TextAreaReadOnly	DontDelete
  rows		KJS::HTMLElement::TextAreaRows		DontDelete
  tabIndex	KJS::HTMLElement::TextAreaTabIndex	DontDelete
  type		KJS::HTMLElement::TextAreaType		DontDelete|ReadOnly
  value		KJS::HTMLElement::TextAreaValue		DontDelete
  blur		KJS::HTMLElement::TextAreaBlur		DontDelete|Function 0
  focus		KJS::HTMLElement::TextAreaFocus		DontDelete|Function 0
  select	KJS::HTMLElement::TextAreaSelect	DontDelete|Function 0
@end
@begin HTMLButtonElementTable 7
  form		KJS::HTMLElement::ButtonForm		DontDelete|ReadOnly
  accessKey	KJS::HTMLElement::ButtonAccessKey	DontDelete
  disabled	KJS::HTMLElement::ButtonDisabled	DontDelete
  name		KJS::HTMLElement::ButtonName		DontDelete
  tabIndex	KJS::HTMLElement::ButtonTabIndex	DontDelete
  type		KJS::HTMLElement::ButtonType		DontDelete|ReadOnly
  value		KJS::HTMLElement::ButtonValue		DontDelete
@end
@begin HTMLLabelElementTable 3
  form		KJS::HTMLElement::LabelForm		DontDelete|ReadOnly
  accessKey	KJS::HTMLElement::LabelAccessKey	DontDelete
  htmlFor	KJS::HTMLElement::LabelHtmlFor		DontDelete
@end
@begin HTMLFieldSetElementTable 1
  form		KJS::HTMLElement::FieldSetForm		DontDelete|ReadOnly
@end
@begin HTMLLegendElementTable 3
  form		KJS::HTMLElement::LegendForm		DontDelete|ReadOnly
  accessKey	KJS::HTMLElement::LegendAccessKey	DontDelete
  align		KJS::HTMLElement::LegendAlign		DontDelete
@end
@begin HTMLUListElementTable 2
  compact	KJS::HTMLElement::UListCompact		DontDelete
  type		KJS::HTMLElement::UListType		DontDelete
@end
@begin HTMLOListElementTable 3
  compact	KJS::HTMLElement::OListCompact		DontDelete
  start		KJS::HTMLElement::OListStart		DontDelete
  type		KJS::HTMLElement::OListType		DontDelete
@end
@begin HTMLDListElementTable 1
  compact	KJS::HTMLElement::DListCompact		DontDelete
@end
@begin HTMLDirectoryElementTable 1
  compact	KJS::HTMLElement::DirectoryCompact	DontDelete
@end
@begin HTMLMenuElementTable 1
  compact	KJS::HTMLElement::MenuCompact		DontDelete
@end
@begin HTMLLIElementTable 2
  type		KJS::HTMLElement::LIType		DontDelete
  value		KJS::HTMLElement::LIValue		DontDelete
@end
@begin HTMLDivElementTable 1
  align		KJS::HTMLElement::DivAlign		DontDelete
@end
@begin HTMLParagraphElementTable 1
  align		KJS::HTMLElement::ParagraphAlign	DontDelete
@end
@begin HTMLHeadingElementTable 1
  align		KJS::HTMLElement::HeadingAlign		DontDelete
@end
@begin HTMLBlockQuoteElementTable 1
  cite		KJS::HTMLElement::BlockQuoteCite	DontDelete
@end
@begin HTMLQuoteElementTable 1
  cite		KJS::HTMLElement::QuoteCite		DontDelete
@end
@begin HTMLPreElementTable 1
  width		KJS::HTMLElement::PreWidth		DontDelete
@end
@begin HTMLBRElementTable 1
  clear		KJS::HTMLElement::BRClear		DontDelete
@end
@begin HTMLBaseFontElementTable 3
  color		KJS::HTMLElement::BaseFontColor		DontDelete
  face		KJS::HTMLElement::BaseFontFace		DontDelete
  size		KJS::HTMLElement::BaseFontSize		DontDelete
@end
@begin HTMLFontElementTable 3
  color		KJS::HTMLElement::FontColor		DontDelete
  face		KJS::HTMLElement::FontFace		DontDelete
  size		KJS::HTMLElement::FontSize		DontDelete
@end
@begin HTMLHRElementTable 4
  align		KJS::HTMLElement::HRAlign		DontDelete
  noShade	KJS::HTMLElement::HRNoShade		DontDelete
  size		KJS::HTMLElement::HRSize		DontDelete
  width		KJS::HTMLElement::HRWidth		DontDelete
@end
@begin HTMLModElementTable 2
  cite		KJS::HTMLElement::ModCite		DontDelete
  dateTime	KJS::HTMLElement::ModDateTime		DontDelete
@end
@begin HTMLAnchorElementTable 23
  accessKey	KJS::HTMLElement::AnchorAccessKey	DontDelete
  charset	KJS::HTMLElement::AnchorCharset		DontDelete
  coords	KJS::HTMLElement::AnchorCoords		DontDelete
  href		KJS::HTMLElement::AnchorHref		DontDelete
  hreflang	KJS::HTMLElement::AnchorHrefLang	DontDelete
  hash		KJS::HTMLElement::AnchorHash		DontDelete|ReadOnly
  host		KJS::HTMLElement::AnchorHost		DontDelete|ReadOnly
  hostname	KJS::HTMLElement::AnchorHostname	DontDelete|ReadOnly
  name		KJS::HTMLElement::AnchorName		DontDelete
  pathname	KJS::HTMLElement::AnchorPathName	DontDelete|ReadOnly
  port		KJS::HTMLElement::AnchorPort		DontDelete|ReadOnly
  protocol	KJS::HTMLElement::AnchorProtocol	DontDelete|ReadOnly
  rel		KJS::HTMLElement::AnchorRel		DontDelete
  rev		KJS::HTMLElement::AnchorRev		DontDelete
  search	KJS::HTMLElement::AnchorSearch		DontDelete|ReadOnly
  shape		KJS::HTMLElement::AnchorShape		DontDelete
  tabIndex	KJS::HTMLElement::AnchorTabIndex	DontDelete
  target	KJS::HTMLElement::AnchorTarget		DontDelete
  text		KJS::HTMLElement::AnchorText		DontDelete|ReadOnly
  type		KJS::HTMLElement::AnchorType		DontDelete
  blur		KJS::HTMLElement::AnchorBlur		DontDelete|Function 0
  focus		KJS::HTMLElement::AnchorFocus		DontDelete|Function 0
@end
@begin HTMLImageElementTable 12
  name		KJS::HTMLElement::ImageName		DontDelete
  align		KJS::HTMLElement::ImageAlign		DontDelete
  alt		KJS::HTMLElement::ImageAlt		DontDelete
  border	KJS::HTMLElement::ImageBorder		DontDelete
  height	KJS::HTMLElement::ImageHeight		DontDelete
  hspace	KJS::HTMLElement::ImageHspace		DontDelete
  isMap		KJS::HTMLElement::ImageIsMap		DontDelete
  longDesc	KJS::HTMLElement::ImageLongDesc		DontDelete
  src		KJS::HTMLElement::ImageSrc		DontDelete
  useMap	KJS::HTMLElement::ImageUseMap		DontDelete
  vspace	KJS::HTMLElement::ImageVspace		DontDelete
  width		KJS::HTMLElement::ImageWidth		DontDelete
@end
@begin HTMLObjectElementTable 20
  form		  KJS::HTMLElement::ObjectForm		  DontDelete|ReadOnly
  code		  KJS::HTMLElement::ObjectCode		  DontDelete
  align		  KJS::HTMLElement::ObjectAlign		  DontDelete
  archive	  KJS::HTMLElement::ObjectArchive	  DontDelete
  border	  KJS::HTMLElement::ObjectBorder	  DontDelete
  codeBase	  KJS::HTMLElement::ObjectCodeBase	  DontDelete
  codeType	  KJS::HTMLElement::ObjectCodeType	  DontDelete
  contentDocument KJS::HTMLElement::ObjectContentDocument DontDelete|ReadOnly
  data		  KJS::HTMLElement::ObjectData		  DontDelete
  declare	  KJS::HTMLElement::ObjectDeclare	  DontDelete
  height	  KJS::HTMLElement::ObjectHeight	  DontDelete
  hspace	  KJS::HTMLElement::ObjectHspace	  DontDelete
  name		  KJS::HTMLElement::ObjectName		  DontDelete
  standby	  KJS::HTMLElement::ObjectStandby	  DontDelete
  tabIndex	  KJS::HTMLElement::ObjectTabIndex	  DontDelete
  type		  KJS::HTMLElement::ObjectType		  DontDelete
  useMap	  KJS::HTMLElement::ObjectUseMap	  DontDelete
  vspace	  KJS::HTMLElement::ObjectVspace	  DontDelete
  width		  KJS::HTMLElement::ObjectWidth		  DontDelete
@end
@begin HTMLParamElementTable 4
  name		KJS::HTMLElement::ParamName		DontDelete
  type		KJS::HTMLElement::ParamType		DontDelete
  value		KJS::HTMLElement::ParamValue		DontDelete
  valueType	KJS::HTMLElement::ParamValueType	DontDelete
@end
@begin HTMLAppletElementTable 11
  align		KJS::HTMLElement::AppletAlign		DontDelete
  alt		KJS::HTMLElement::AppletAlt		DontDelete
  archive	KJS::HTMLElement::AppletArchive		DontDelete
  code		KJS::HTMLElement::AppletCode		DontDelete
  codeBase	KJS::HTMLElement::AppletCodeBase	DontDelete
  height	KJS::HTMLElement::AppletHeight		DontDelete
  hspace	KJS::HTMLElement::AppletHspace		DontDelete
  name		KJS::HTMLElement::AppletName		DontDelete
  object	KJS::HTMLElement::AppletObject		DontDelete
  vspace	KJS::HTMLElement::AppletVspace		DontDelete
  width		KJS::HTMLElement::AppletWidth		DontDelete
@end
@begin HTMLMapElementTable 2
  areas		KJS::HTMLElement::MapAreas		DontDelete|ReadOnly
  name		KJS::HTMLElement::MapName		DontDelete
@end
@begin HTMLAreaElementTable 15
  accessKey	KJS::HTMLElement::AreaAccessKey		DontDelete
  alt		KJS::HTMLElement::AreaAlt		DontDelete
  coords	KJS::HTMLElement::AreaCoords		DontDelete
  href		KJS::HTMLElement::AreaHref		DontDelete
  hash		KJS::HTMLElement::AreaHash		DontDelete|ReadOnly
  host		KJS::HTMLElement::AreaHost		DontDelete|ReadOnly
  hostname	KJS::HTMLElement::AreaHostName		DontDelete|ReadOnly
  pathname	KJS::HTMLElement::AreaPathName		DontDelete|ReadOnly
  port		KJS::HTMLElement::AreaPort		DontDelete|ReadOnly
  protocol	KJS::HTMLElement::AreaProtocol		DontDelete|ReadOnly
  search	KJS::HTMLElement::AreaSearch		DontDelete|ReadOnly
  noHref	KJS::HTMLElement::AreaNoHref		DontDelete
  shape		KJS::HTMLElement::AreaShape		DontDelete
  tabIndex	KJS::HTMLElement::AreaTabIndex		DontDelete
  target	KJS::HTMLElement::AreaTarget		DontDelete
@end
@begin HTMLScriptElementTable 7
  text		KJS::HTMLElement::ScriptText		DontDelete
  htmlFor	KJS::HTMLElement::ScriptHtmlFor		DontDelete
  event		KJS::HTMLElement::ScriptEvent		DontDelete
  charset	KJS::HTMLElement::ScriptCharset		DontDelete
  defer		KJS::HTMLElement::ScriptDefer		DontDelete
  src		KJS::HTMLElement::ScriptSrc		DontDelete
  type		KJS::HTMLElement::ScriptType		DontDelete
@end
@begin HTMLTableElementTable 23
  caption	KJS::HTMLElement::TableCaption		DontDelete
  tHead		KJS::HTMLElement::TableTHead		DontDelete
  tFoot		KJS::HTMLElement::TableTFoot		DontDelete
  rows		KJS::HTMLElement::TableRows		DontDelete|ReadOnly
  tBodies	KJS::HTMLElement::TableTBodies		DontDelete|ReadOnly
  align		KJS::HTMLElement::TableAlign		DontDelete
  bgColor	KJS::HTMLElement::TableBgColor		DontDelete
  border	KJS::HTMLElement::TableBorder		DontDelete
  cellPadding	KJS::HTMLElement::TableCellPadding	DontDelete
  cellSpacing	KJS::HTMLElement::TableCellSpacing	DontDelete
  frame		KJS::HTMLElement::TableFrame		DontDelete
  rules		KJS::HTMLElement::TableRules		DontDelete
  summary	KJS::HTMLElement::TableSummary		DontDelete
  width		KJS::HTMLElement::TableWidth		DontDelete
  createTHead	KJS::HTMLElement::TableCreateTHead	DontDelete|Function 0
  deleteTHead	KJS::HTMLElement::TableDeleteTHead	DontDelete|Function 0
  createTFoot	KJS::HTMLElement::TableCreateTFoot	DontDelete|Function 0
  deleteTFoot	KJS::HTMLElement::TableDeleteTFoot	DontDelete|Function 0
  createCaption	KJS::HTMLElement::TableCreateCaption	DontDelete|Function 0
  deleteCaption	KJS::HTMLElement::TableDeleteCaption	DontDelete|Function 0
  insertRow	KJS::HTMLElement::TableInsertRow	DontDelete|Function 1
  deleteRow	KJS::HTMLElement::TableDeleteRow	DontDelete|Function 1
@end
@begin HTMLTableCaptionElementTable 1
  align		KJS::HTMLElement::TableCaptionAlign	DontDelete
@end
@begin HTMLTableColElementTable 7
  align		KJS::HTMLElement::TableColAlign		DontDelete
  ch		KJS::HTMLElement::TableColCh		DontDelete
  chOff		KJS::HTMLElement::TableColChOff		DontDelete
  span		KJS::HTMLElement::TableColSpan		DontDelete
  vAlign	KJS::HTMLElement::TableColVAlign	DontDelete
  width		KJS::HTMLElement::TableColWidth		DontDelete
@end
@begin HTMLTableSectionElementTable 7
  align		KJS::HTMLElement::TableSectionAlign		DontDelete
  ch		KJS::HTMLElement::TableSectionCh		DontDelete
  chOff		KJS::HTMLElement::TableSectionChOff		DontDelete
  vAlign	KJS::HTMLElement::TableSectionVAlign		DontDelete
  rows		KJS::HTMLElement::TableSectionRows		DontDelete|ReadOnly
  insertRow	KJS::HTMLElement::TableSectionInsertRow		DontDelete|Function 1
  deleteRow	KJS::HTMLElement::TableSectionDeleteRow		DontDelete|Function 1
@end
@begin HTMLTableRowElementTable 11
  rowIndex	KJS::HTMLElement::TableRowRowIndex		DontDelete|ReadOnly
  sectionRowIndex KJS::HTMLElement::TableRowSectionRowIndex	DontDelete|ReadOnly
  cells		KJS::HTMLElement::TableRowCells			DontDelete|ReadOnly
  align		KJS::HTMLElement::TableRowAlign			DontDelete
  bgColor	KJS::HTMLElement::TableRowBgColor		DontDelete
  ch		KJS::HTMLElement::TableRowCh			DontDelete
  chOff		KJS::HTMLElement::TableRowChOff			DontDelete
  vAlign	KJS::HTMLElement::TableRowVAlign		DontDelete
  insertCell	KJS::HTMLElement::TableRowInsertCell		DontDelete|Function 1
  deleteCell	KJS::HTMLElement::TableRowDeleteCell		DontDelete|Function 1
@end
@begin HTMLTableCellElementTable 15
  cellIndex	KJS::HTMLElement::TableCellCellIndex		DontDelete|ReadOnly
  abbr		KJS::HTMLElement::TableCellAbbr			DontDelete
  align		KJS::HTMLElement::TableCellAlign		DontDelete
  axis		KJS::HTMLElement::TableCellAxis			DontDelete
  bgColor	KJS::HTMLElement::TableCellBgColor		DontDelete
  ch		KJS::HTMLElement::TableCellCh			DontDelete
  chOff		KJS::HTMLElement::TableCellChOff		DontDelete
  colSpan	KJS::HTMLElement::TableCellColSpan		DontDelete
  headers	KJS::HTMLElement::TableCellHeaders		DontDelete
  height	KJS::HTMLElement::TableCellHeight		DontDelete
  noWrap	KJS::HTMLElement::TableCellNoWrap		DontDelete
  rowSpan	KJS::HTMLElement::TableCellRowSpan		DontDelete
  scope		KJS::HTMLElement::TableCellScope		DontDelete
  vAlign	KJS::HTMLElement::TableCellVAlign		DontDelete
  width		KJS::HTMLElement::TableCellWidth		DontDelete
@end
@begin HTMLFrameSetElementTable 2
  cols		KJS::HTMLElement::FrameSetCols			DontDelete
  rows		KJS::HTMLElement::FrameSetRows			DontDelete
@end
@begin HTMLFrameElementTable 9
  contentDocument KJS::HTMLElement::FrameContentDocument        DontDelete|ReadOnly
  frameBorder     KJS::HTMLElement::FrameFrameBorder		DontDelete
  longDesc	  KJS::HTMLElement::FrameLongDesc		DontDelete
  marginHeight	  KJS::HTMLElement::FrameMarginHeight		DontDelete
  marginWidth	  KJS::HTMLElement::FrameMarginWidth		DontDelete
  name		  KJS::HTMLElement::FrameName			DontDelete
  noResize	  KJS::HTMLElement::FrameNoResize		DontDelete
  scrolling	  KJS::HTMLElement::FrameScrolling		DontDelete
  src		  KJS::HTMLElement::FrameSrc			DontDelete
  location	  KJS::HTMLElement::FrameLocation		DontDelete
@end
@begin HTMLIFrameElementTable 12
  align		  KJS::HTMLElement::IFrameAlign			DontDelete
  contentDocument KJS::HTMLElement::IFrameContentDocument       DontDelete|ReadOnly
  document	  KJS::HTMLElement::IFrameDocument		DontDelete|ReadOnly
  frameBorder	  KJS::HTMLElement::IFrameFrameBorder		DontDelete
  height	  KJS::HTMLElement::IFrameHeight		DontDelete
  longDesc	  KJS::HTMLElement::IFrameLongDesc		DontDelete
  marginHeight	  KJS::HTMLElement::IFrameMarginHeight		DontDelete
  marginWidth	  KJS::HTMLElement::IFrameMarginWidth		DontDelete
  name		  KJS::HTMLElement::IFrameName			DontDelete
  scrolling	  KJS::HTMLElement::IFrameScrolling		DontDelete
  src		  KJS::HTMLElement::IFrameSrc			DontDelete
  width		  KJS::HTMLElement::IFrameWidth			DontDelete
@end
*/

Value KJS::HTMLElement::tryGet(ExecState *exec, const UString &propertyName) const
{
  DOM::HTMLElement element = static_cast<DOM::HTMLElement>(node);
#ifdef KJS_VERBOSE
  kdDebug(6070) << "KJS::HTMLElement::tryGet " << propertyName.qstring() << " thisTag=" << element.tagName().string() << endl;
#endif
  // First look at dynamic properties
  switch (element.elementId()) {
    case ID_FORM: {
      DOM::HTMLFormElement form = element;
      // Check if we're retrieving an element (by index or by name)
      bool ok;
      uint u = propertyName.toULong(&ok);
      if (ok)
        return getDOMNode(exec,form.elements().item(u));
      KJS::HTMLCollection coll(exec,form.elements());
      Value namedItems = coll.getNamedItems(exec, propertyName);
      if (namedItems.type() != UndefinedType)
        return namedItems;
    }
      break;
    case ID_SELECT: {
      DOM::HTMLSelectElement select = element;
      bool ok;
      uint u = propertyName.toULong(&ok);
      if (ok)
        return getDOMNode(exec,select.options().item(u)); // not specified by DOM(?) but supported in netscape/IE
    }
      break;
  case ID_FRAME:
  case ID_IFRAME: {
      DOM::DocumentImpl* doc = static_cast<DOM::HTMLFrameElementImpl *>(element.handle())->contentDocument();
      if ( doc && doc->view() ) {
        KHTMLPart* part = doc->view()->part();
        if ( part ) {
          Object globalObject = Object::dynamicCast( Window::retrieve( part ) );
          // Calling hasProperty on a Window object doesn't work, it always says true.
          // Hence we need to use getDirect instead.
          if ( !globalObject.isNull() && static_cast<ObjectImp *>(globalObject.imp())->getDirect( propertyName ) )
            return globalObject.get( exec, propertyName );
        }
      }
  }
  default:
    break;
  }

  const HashTable* table = classInfo()->propHashTable; // get the right hashtable
  const HashEntry* entry = Lookup::findEntry(table, propertyName);
  if (entry) {
    if (entry->attr & Function)
      return lookupOrCreateFunction<KJS::HTMLElementFunction>(exec, propertyName, this, entry->value, entry->params, entry->attr);
    return getValueProperty(exec, entry->value);
  }

  // Base HTMLElement stuff or parent class forward, as usual
  return DOMObjectLookupGet<KJS::HTMLElementFunction, KJS::HTMLElement, DOMElement>(exec, propertyName, &KJS::HTMLElementTable, this);
}

Value KJS::HTMLElement::getValueProperty(ExecState *exec, int token) const
{
  DOM::HTMLElement element = static_cast<DOM::HTMLElement>(node);
  switch (element.elementId()) {
  case ID_HTML: {
    DOM::HTMLHtmlElement html = element;
    if      (token == HtmlVersion)         return getString(html.version());
  }
  break;
  case ID_HEAD: {
    DOM::HTMLHeadElement head = element;
    if      (token == HeadProfile)         return getString(head.profile());
  }
  break;
  case ID_LINK: {
    DOM::HTMLLinkElement link = element;
    switch (token) {
    case LinkDisabled:        return Boolean(link.disabled());
    case LinkCharset:         return getString(link.charset());
    case LinkHref:            return getString(link.href());
    case LinkHrefLang:        return getString(link.hreflang());
    case LinkMedia:           return getString(link.media());
    case LinkRel:             return getString(link.rel());
    case LinkRev:             return getString(link.rev());
    case LinkTarget:          return getString(link.target());
    case LinkType:            return getString(link.type());
    case LinkSheet:           return getDOMStyleSheet(exec,static_cast<DOM::ProcessingInstruction>(node).sheet());
    }
  }
  break;
  case ID_TITLE: {
    DOM::HTMLTitleElement title = element;
    switch (token) {
    case TitleText:                 return getString(title.text());
    }
  }
  break;
  case ID_META: {
    DOM::HTMLMetaElement meta = element;
    switch (token) {
    case MetaContent:         return getString(meta.content());
    case MetaHttpEquiv:       return getString(meta.httpEquiv());
    case MetaName:            return getString(meta.name());
    case MetaScheme:          return getString(meta.scheme());
    }
  }
  break;
  case ID_BASE: {
    DOM::HTMLBaseElement base = element;
    switch (token) {
    case BaseHref:            return getString(base.href());
    case BaseTarget:          return getString(base.target());
    }
  }
  break;
  case ID_ISINDEX: {
    DOM::HTMLIsIndexElement isindex = element;
    switch (token) {
    case IsIndexForm:            return getDOMNode(exec,isindex.form()); // type HTMLFormElement
    case IsIndexPrompt:          return getString(isindex.prompt());
    }
  }
  break;
  case ID_STYLE: {
    DOM::HTMLStyleElement style = element;
    switch (token) {
    case StyleDisabled:        return Boolean(style.disabled());
    case StyleMedia:           return getString(style.media());
    case StyleType:            return getString(style.type());
    case StyleSheet:           return getDOMStyleSheet(exec,static_cast<DOM::ProcessingInstruction>(node).sheet());
    }
  }
  break;
  case ID_BODY: {
    DOM::HTMLBodyElement body = element;
    switch (token) {
    case BodyALink:           return getString(body.aLink());
    case BodyBackground:      return getString(body.background());
    case BodyBgColor:         return getString(body.bgColor());
    case BodyLink:            return getString(body.link());
    case BodyText:            return getString(body.text());
    case BodyVLink:           return getString(body.vLink());
    case BodyScrollHeight:   return Number(body.ownerDocument().view() ? body.ownerDocument().view()->contentsHeight() : 0);
    case BodyScrollWidth:    return Number(body.ownerDocument().view() ? body.ownerDocument().view()->contentsWidth() : 0);
    }
  }
  break;

  case ID_FORM: {
    DOM::HTMLFormElement form = element;
    switch (token) {
    case FormElements:        return getHTMLCollection(exec,form.elements());
    case FormLength:          return Number(form.length());
    case FormName:            return getString(form.name());
    case FormAcceptCharset:   return getString(form.acceptCharset());
    case FormAction:          return getString(form.action());
    case FormEncType:         return getString(form.enctype());
    case FormMethod:          return getString(form.method());
    case FormTarget:          return getString(form.target());
    }
  }
  break;
  case ID_SELECT: {
    DOM::HTMLSelectElement select = element;
    switch (token) {
    case SelectType:            return getString(select.type());
    case SelectSelectedIndex:   return Number(select.selectedIndex());
    case SelectValue:           return getString(select.value());
    case SelectLength:          return Number(select.length());
    case SelectForm:            return getDOMNode(exec,select.form()); // type HTMLFormElement
    case SelectOptions:         return getSelectHTMLCollection(exec, select.options(), select); // type HTMLCollection
    case SelectDisabled:        return Boolean(select.disabled());
    case SelectMultiple:        return Boolean(select.multiple());
    case SelectName:            return getString(select.name());
    case SelectSize:            return Number(select.size());
    case SelectTabIndex:        return Number(select.tabIndex());
    }
  }
  break;
  case ID_OPTGROUP: {
    DOM::HTMLOptGroupElement optgroup = element;
    switch (token) {
    case OptGroupDisabled:        return Boolean(optgroup.disabled());
    case OptGroupLabel:           return getString(optgroup.label());
    }
  }
  break;
  case ID_OPTION: {
    DOM::HTMLOptionElement option = element;
    switch (token) {
    case OptionForm:            return getDOMNode(exec,option.form()); // type HTMLFormElement
    case OptionDefaultSelected: return Boolean(option.defaultSelected());
    case OptionText:            return getString(option.text());
    case OptionIndex:           return Number(option.index());
    case OptionDisabled:        return Boolean(option.disabled());
    case OptionLabel:           return getString(option.label());
    case OptionSelected:        return Boolean(option.selected());
    case OptionValue:           return getString(option.value());
    }
  }
  break;
  case ID_INPUT: {
    DOM::HTMLInputElement input = element;
    switch (token) {
    case InputDefaultValue:    return getString(input.defaultValue());
    case InputDefaultChecked:  return Boolean(input.defaultChecked());
    case InputForm:            return getDOMNode(exec,input.form()); // type HTMLFormElement
    case InputAccept:          return getString(input.accept());
    case InputAccessKey:       return getString(input.accessKey());
    case InputAlign:           return getString(input.align());
    case InputAlt:             return getString(input.alt());
    case InputChecked:         return Boolean(input.checked());
    case InputDisabled:        return Boolean(input.disabled());
    case InputMaxLength:       return Number(input.maxLength());
    case InputName:            return getString(input.name());
    case InputReadOnly:        return Boolean(input.readOnly());
    case InputSize:            return getString(input.size());
    case InputSrc:             return getString(input.src());
    case InputTabIndex:        return Number(input.tabIndex());
    case InputType:            return getString(input.type());
    case InputUseMap:          return getString(input.useMap());
    case InputValue:           return getString(input.value());
    }
  }
  break;
  case ID_TEXTAREA: {
    DOM::HTMLTextAreaElement textarea = element;
    switch (token) {
    case TextAreaDefaultValue:    return getString(textarea.defaultValue());
    case TextAreaForm:            return getDOMNode(exec,textarea.form()); // type HTMLFormElement
    case TextAreaAccessKey:       return getString(textarea.accessKey());
    case TextAreaCols:            return Number(textarea.cols());
    case TextAreaDisabled:        return Boolean(textarea.disabled());
    case TextAreaName:            return getString(textarea.name());
    case TextAreaReadOnly:        return Boolean(textarea.readOnly());
    case TextAreaRows:            return Number(textarea.rows());
    case TextAreaTabIndex:        return Number(textarea.tabIndex());
    case TextAreaType:            return getString(textarea.type());
    case TextAreaValue:           return getString(textarea.value());
    }
  }
  break;
  case ID_BUTTON: {
    DOM::HTMLButtonElement button = element;
    switch (token) {
    case ButtonForm:            return getDOMNode(exec,button.form()); // type HTMLFormElement
    case ButtonAccessKey:       return getString(button.accessKey());
    case ButtonDisabled:        return Boolean(button.disabled());
    case ButtonName:            return getString(button.name());
    case ButtonTabIndex:        return Number(button.tabIndex());
    case ButtonType:            return getString(button.type());
    case ButtonValue:           return getString(button.value());
    }
  }
  break;
  case ID_LABEL: {
    DOM::HTMLLabelElement label = element;
    switch (token) {
    case LabelForm:            return getDOMNode(exec,label.form()); // type HTMLFormElement
    case LabelAccessKey:       return getString(label.accessKey());
    case LabelHtmlFor:         return getString(label.htmlFor());
    }
  }
  break;
  case ID_FIELDSET: {
    DOM::HTMLFieldSetElement fieldSet = element;
    switch (token) {
    case FieldSetForm:            return getDOMNode(exec,fieldSet.form()); // type HTMLFormElement
    }
  }
  break;
  case ID_LEGEND: {
    DOM::HTMLLegendElement legend = element;
    switch (token) {
    case LegendForm:            return getDOMNode(exec,legend.form()); // type HTMLFormElement
    case LegendAccessKey:       return getString(legend.accessKey());
    case LegendAlign:           return getString(legend.align());
    }
  }
  break;
  case ID_UL: {
    DOM::HTMLUListElement uList = element;
    switch (token) {
    case UListCompact:         return Boolean(uList.compact());
    case UListType:            return getString(uList.type());
    }
  }
  break;
  case ID_OL: {
    DOM::HTMLOListElement oList = element;
    switch (token) {
    case OListCompact:         return Boolean(oList.compact());
    case OListStart:           return Number(oList.start());
    case OListType:            return getString(oList.type());
    }
  }
  break;
  case ID_DL: {
    DOM::HTMLDListElement dList = element;
    switch (token) {
    case DListCompact:         return Boolean(dList.compact());
    }
  }
  break;
  case ID_DIR: {
    DOM::HTMLDirectoryElement directory = element;
    switch (token) {
    case DirectoryCompact:         return Boolean(directory.compact());
    }
  }
  break;
  case ID_MENU: {
    DOM::HTMLMenuElement menu = element;
    switch (token) {
    case MenuCompact:         return Boolean(menu.compact());
    }
  }
  break;
  case ID_LI: {
    DOM::HTMLLIElement li = element;
    switch (token) {
    case LIType:            return getString(li.type());
    case LIValue:           return Number(li.value());
    }
  }
  break;
  case ID_DIV: {
    DOM::HTMLDivElement div = element;
    switch (token) {
    case DivAlign:           return getString(div.align());
    }
  }
  break;
  case ID_P: {
    DOM::HTMLParagraphElement paragraph = element;
    switch (token) {
    case ParagraphAlign:           return getString(paragraph.align());
    }
  }
  break;
  case ID_H1:
  case ID_H2:
  case ID_H3:
  case ID_H4:
  case ID_H5:
  case ID_H6: {
    DOM::HTMLHeadingElement heading = element;
    switch (token) {
    case HeadingAlign:           return getString(heading.align());
    }
  }
  break;
  case ID_BLOCKQUOTE: {
    DOM::HTMLBlockquoteElement blockquote = element;
    switch (token) {
    case BlockQuoteCite:            return getString(blockquote.cite());
    }
  }
  case ID_Q: {
    DOM::HTMLQuoteElement quote = element;
    switch (token) {
    case QuoteCite:            return getString(quote.cite());
    }
  }
  case ID_PRE: {
    DOM::HTMLPreElement pre = element;
    switch (token) {
    case PreWidth:           return Number(pre.width());
    }
  }
  break;
  case ID_BR: {
    DOM::HTMLBRElement br = element;
    switch (token) {
    case BRClear:           return getString(br.clear());
    }
  }
  break;
  case ID_BASEFONT: {
    DOM::HTMLBaseFontElement baseFont = element;
    switch (token) {
    case BaseFontColor:           return getString(baseFont.color());
    case BaseFontFace:            return getString(baseFont.face());
    case BaseFontSize:            return getString(baseFont.size());
    }
  }
  break;
  case ID_FONT: {
    DOM::HTMLFontElement font = element;
    switch (token) {
    case FontColor:           return getString(font.color());
    case FontFace:            return getString(font.face());
    case FontSize:            return getString(font.size());
    }
  }
  break;
  case ID_HR: {
    DOM::HTMLHRElement hr = element;
    switch (token) {
    case HRAlign:           return getString(hr.align());
    case HRNoShade:         return Boolean(hr.noShade());
    case HRSize:            return getString(hr.size());
    case HRWidth:           return getString(hr.width());
    }
  }
  break;
  case ID_INS:
  case ID_DEL: {
    DOM::HTMLModElement mod = element;
    switch (token) {
    case ModCite:            return getString(mod.cite());
    case ModDateTime:        return getString(mod.dateTime());
    }
  }
  break;
  case ID_A: {
    DOM::HTMLAnchorElement anchor = element;
    switch (token) {
    case AnchorAccessKey:       return getString(anchor.accessKey());
    case AnchorCharset:         return getString(anchor.charset());
    case AnchorCoords:          return getString(anchor.coords());
    case AnchorHref:            return getString(anchor.href());
    case AnchorHrefLang:        return getString(anchor.hreflang());
    case AnchorHash:            return getString('#'+KURL(anchor.href().string()).ref());
    case AnchorHost:            return getString(KURL(anchor.href().string()).host());
    case AnchorHostname: {
      KURL url(anchor.href().string());
      kdDebug(6070) << "anchor::hostname uses:" <<url.url()<<endl;
      if (url.port()==0)
        return getString(url.host());
      else
        return getString(url.host() + ":" + QString::number(url.port()));
    }
    case AnchorPathName:        return getString(KURL(anchor.href().string()).path());
    case AnchorPort:            return getString(QString::number(KURL(anchor.href().string()).port()));
    case AnchorProtocol:        return getString(KURL(anchor.href().string()).protocol()+":");
    case AnchorSearch:          return getString(KURL(anchor.href().string()).query());
    case AnchorName:            return getString(anchor.name());
    case AnchorRel:             return getString(anchor.rel());
    case AnchorRev:             return getString(anchor.rev());
    case AnchorShape:           return getString(anchor.shape());
    case AnchorTabIndex:        return Number(anchor.tabIndex());
    case AnchorTarget:          return getString(anchor.target());
    case AnchorText:            return getString(anchor.innerHTML());
    case AnchorType:            return getString(anchor.type());
    }
  }
  break;
  case ID_IMG: {
    DOM::HTMLImageElement image = element;
    switch (token) {
    case ImageName:            return getString(image.name());
    case ImageAlign:           return getString(image.align());
    case ImageAlt:             return getString(image.alt());
    case ImageBorder:          return Number(image.border());
    case ImageHeight:          return Number(image.height());
    case ImageHspace:          return Number(image.hspace());
    case ImageIsMap:           return Boolean(image.isMap());
    case ImageLongDesc:        return getString(image.longDesc());
    case ImageSrc:             return getString(image.src());
    case ImageUseMap:          return getString(image.useMap());
    case ImageVspace:          return Number(image.vspace());
    case ImageWidth:           return Number(image.width());
    }
  }
  break;
  case ID_OBJECT: {
    DOM::HTMLObjectElement object = element;
    switch (token) {
    case ObjectForm:            return getDOMNode(exec,object.form()); // type HTMLFormElement
    case ObjectCode:            return getString(object.code());
    case ObjectAlign:           return getString(object.align());
    case ObjectArchive:         return getString(object.archive());
    case ObjectBorder:          return getString(object.border());
    case ObjectCodeBase:        return getString(object.codeBase());
    case ObjectCodeType:        return getString(object.codeType());
    case ObjectContentDocument: return getDOMNode(exec, object.contentDocument());
    case ObjectData:            return getString(object.data());
    case ObjectDeclare:         return Boolean(object.declare());
    case ObjectHeight:          return getString(object.height());
    case ObjectHspace:          return getString(object.hspace());
    case ObjectName:            return getString(object.name());
    case ObjectStandby:         return getString(object.standby());
    case ObjectTabIndex:        return Number(object.tabIndex());
    case ObjectType:            return getString(object.type());
    case ObjectUseMap:          return getString(object.useMap());
    case ObjectVspace:          return getString(object.vspace());
    case ObjectWidth:           return getString(object.width());
    }
  }
  break;
  case ID_PARAM: {
    DOM::HTMLParamElement param = element;
    switch (token) {
    case ParamName:            return getString(param.name());
    case ParamType:            return getString(param.type());
    case ParamValue:           return getString(param.value());
    case ParamValueType:       return getString(param.valueType());
    }
  }
  break;
  case ID_APPLET: {
    DOM::HTMLAppletElement applet = element;
    switch (token) {
    case AppletAlign:           return getString(applet.align());
    case AppletAlt:             return getString(applet.alt());
    case AppletArchive:         return getString(applet.archive());
    case AppletCode:            return getString(applet.code());
    case AppletCodeBase:        return getString(applet.codeBase());
    case AppletHeight:          return getString(applet.height());
    case AppletHspace:          return getString(applet.hspace());
    case AppletName:            return getString(applet.name());
    case AppletObject:          return getString(applet.object());
    case AppletVspace:          return getString(applet.vspace());
    case AppletWidth:           return getString(applet.width());
    }
  }
  break;
  case ID_MAP: {
    DOM::HTMLMapElement map = element;
    switch (token) {
    case MapAreas:           return getHTMLCollection(exec, map.areas()); // type HTMLCollection
    case MapName:            return getString(map.name());
    }
  }
  break;
  case ID_AREA: {
    DOM::HTMLAreaElement area = element;
    switch (token) {
    case AreaAccessKey:       return getString(area.accessKey());
    case AreaAlt:             return getString(area.alt());
    case AreaCoords:          return getString(area.coords());
    case AreaHref:            return getString(area.href());
    case AreaHash:            return getString('#'+KURL(area.href().string()).ref());
    case AreaHost:            return getString(KURL(area.href().string()).host());
    case AreaHostName: {
      KURL url(area.href().string());
      kdDebug(6070) << "link::hostname uses:" <<url.url()<<endl;
      if (url.port()==0)
        return getString(url.host());
      else
        return getString(url.host() + ":" + QString::number(url.port()));
    }
    case AreaPathName:        return getString(KURL(area.href().string()).path());
    case AreaPort:            return getString(QString::number(KURL(area.href().string()).port()));
    case AreaProtocol:        return getString(KURL(area.href().string()).protocol()+":");
    case AreaSearch:          return getString(KURL(area.href().string()).query());
    case AreaNoHref:          return Boolean(area.noHref());
    case AreaShape:           return getString(area.shape());
    case AreaTabIndex:        return Number(area.tabIndex());
    case AreaTarget:          return getString(area.target());
    }
  }
  break;
  case ID_SCRIPT: {
    DOM::HTMLScriptElement script = element;
    switch (token) {
    case ScriptText:            return getString(script.text());
    case ScriptHtmlFor:         return getString(script.htmlFor());
    case ScriptEvent:           return getString(script.event());
    case ScriptCharset:         return getString(script.charset());
    case ScriptDefer:           return Boolean(script.defer());
    case ScriptSrc:             return getString(script.src());
    case ScriptType:            return getString(script.type());
    }
  }
  break;
  case ID_TABLE: {
    DOM::HTMLTableElement table = element;
    switch (token) {
    case TableCaption:         return getDOMNode(exec,table.caption()); // type HTMLTableCaptionElement
    case TableTHead:           return getDOMNode(exec,table.tHead()); // type HTMLTableSectionElement
    case TableTFoot:           return getDOMNode(exec,table.tFoot()); // type HTMLTableSectionElement
    case TableRows:            return getHTMLCollection(exec,table.rows()); // type HTMLCollection
    case TableTBodies:         return getHTMLCollection(exec,table.tBodies()); // type HTMLCollection
    case TableAlign:           return getString(table.align());
    case TableBgColor:         return getString(table.bgColor());
    case TableBorder:          return getString(table.border());
    case TableCellPadding:     return getString(table.cellPadding());
    case TableCellSpacing:     return getString(table.cellSpacing());
    case TableFrame:           return getString(table.frame());
    case TableRules:           return getString(table.rules());
    case TableSummary:         return getString(table.summary());
    case TableWidth:           return getString(table.width());
    }
  }
  break;
  case ID_CAPTION: {
    DOM::HTMLTableCaptionElement tableCaption;
    switch (token) {
    case TableCaptionAlign:       return getString(tableCaption.align());
    }
  }
  break;
  case ID_COL: {
    DOM::HTMLTableColElement tableCol = element;
    switch (token) {
    case TableColAlign:           return getString(tableCol.align());
    case TableColCh:              return getString(tableCol.ch());
    case TableColChOff:           return getString(tableCol.chOff());
    case TableColSpan:            return Number(tableCol.span());
    case TableColVAlign:          return getString(tableCol.vAlign());
    case TableColWidth:           return getString(tableCol.width());
    }
  }
  break;
  case ID_THEAD:
  case ID_TBODY:
  case ID_TFOOT: {
    DOM::HTMLTableSectionElement tableSection = element;
    switch (token) {
    case TableSectionAlign:           return getString(tableSection.align());
    case TableSectionCh:              return getString(tableSection.ch());
    case TableSectionChOff:           return getString(tableSection.chOff());
    case TableSectionVAlign:          return getString(tableSection.vAlign());
    case TableSectionRows:            return getHTMLCollection(exec,tableSection.rows()); // type HTMLCollection
    }
  }
  break;
  case ID_TR: {
   DOM::HTMLTableRowElement tableRow = element;
   switch (token) {
   case TableRowRowIndex:        return Number(tableRow.rowIndex());
   case TableRowSectionRowIndex: return Number(tableRow.sectionRowIndex());
   case TableRowCells:           return getHTMLCollection(exec,tableRow.cells()); // type HTMLCollection
   case TableRowAlign:           return getString(tableRow.align());
   case TableRowBgColor:         return getString(tableRow.bgColor());
   case TableRowCh:              return getString(tableRow.ch());
   case TableRowChOff:           return getString(tableRow.chOff());
   case TableRowVAlign:          return getString(tableRow.vAlign());
   }
  }
  break;
  case ID_TH:
  case ID_TD: {
    DOM::HTMLTableCellElement tableCell = element;
    switch (token) {
    case TableCellCellIndex:       return Number(tableCell.cellIndex());
    case TableCellAbbr:            return getString(tableCell.abbr());
    case TableCellAlign:           return getString(tableCell.align());
    case TableCellAxis:            return getString(tableCell.axis());
    case TableCellBgColor:         return getString(tableCell.bgColor());
    case TableCellCh:              return getString(tableCell.ch());
    case TableCellChOff:           return getString(tableCell.chOff());
    case TableCellColSpan:         return Number(tableCell.colSpan());
    case TableCellHeaders:         return getString(tableCell.headers());
    case TableCellHeight:          return getString(tableCell.height());
    case TableCellNoWrap:          return Boolean(tableCell.noWrap());
    case TableCellRowSpan:         return Number(tableCell.rowSpan());
    case TableCellScope:           return getString(tableCell.scope());
    case TableCellVAlign:          return getString(tableCell.vAlign());
    case TableCellWidth:           return getString(tableCell.width());
    }
  }
  break;
  case ID_FRAMESET: {
    DOM::HTMLFrameSetElement frameSet = element;
    switch (token) {
    case FrameSetCols:            return getString(frameSet.cols());
    case FrameSetRows:            return getString(frameSet.rows());
    }
  }
  break;
  case ID_FRAME: {
    DOM::HTMLFrameElement frameElement = element;
    switch (token) {
    case FrameContentDocument: return getDOMNode(exec, frameElement.contentDocument());
    case FrameFrameBorder:     return getString(frameElement.frameBorder());
    case FrameLongDesc:        return getString(frameElement.longDesc());
    case FrameMarginHeight:    return getString(frameElement.marginHeight());
    case FrameMarginWidth:     return getString(frameElement.marginWidth());
    case FrameName:            return getString(frameElement.name());
    case FrameNoResize:        return Boolean(frameElement.noResize());
    case FrameScrolling:       return getString(frameElement.scrolling());
    case FrameSrc:             return getString(frameElement.src());
    }
  }
  break;
  case ID_IFRAME: {
    DOM::HTMLIFrameElement iFrame = element;
    switch (token) {
    case IFrameAlign:                return getString(iFrame.align());
      // ### security check ?
    case IFrameDocument: // non-standard, mapped to contentDocument
    case IFrameContentDocument:      return getDOMNode(exec, iFrame.contentDocument());
    case IFrameFrameBorder:     return getString(iFrame.frameBorder());
    case IFrameHeight:          return getString(iFrame.height());
    case IFrameLongDesc:        return getString(iFrame.longDesc());
    case IFrameMarginHeight:    return getString(iFrame.marginHeight());
    case IFrameMarginWidth:     return getString(iFrame.marginWidth());
    case IFrameName:            return getString(iFrame.name());
    case IFrameScrolling:       return getString(iFrame.scrolling());
    case IFrameSrc:             return getString(iFrame.src());
    case IFrameWidth:           return getString(iFrame.width());
    }
    break;
  }
  } // xemacs (or arnt) could be a bit smarter when it comes to indenting switch()es ;)
  // its not arnt to blame - its the original Stroustrup style we like :) (Dirk)

  // generic properties
  switch (token) {
  case ElementId:
    return getString(element.id());
  case ElementTitle:
    return getString(element.title());
  case ElementLang:
    return getString(element.lang());
  case ElementDir:
    return getString(element.dir());
  case ElementClassName:
    return getString(element.className());
  case ElementInnerHTML:
    return getString(element.innerHTML());
  case ElementInnerText:
    return getString(element.innerText());
  case ElementDocument:
    return getDOMNode(exec,element.ownerDocument());
  case ElementChildren:
    return getHTMLCollection(exec,element.children());
  // ### what about style? or is this used instead for DOM2 stylesheets?
  }
  kdWarning() << "HTMLElement::getValueProperty unhandled token " << token << endl;
  return Undefined();
}

bool KJS::HTMLElement::hasProperty(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  //kdDebug(6070) << "HTMLElement::hasProperty " << propertyName.qstring() << endl;
#endif
  DOM::HTMLElement element = static_cast<DOM::HTMLElement>(node);
  // First look at dynamic properties - keep this in sync with tryGet
  switch (element.elementId()) {
    case ID_FORM: {
      DOM::HTMLFormElement form = element;
      // Check if we're retrieving an element (by index or by name)
      bool ok;
      uint u = propertyName.toULong(&ok);
      if (ok && !(form.elements().item(u).isNull()))
        return true;
      DOM::Node testnode = form.elements().namedItem(propertyName.string());
      if (!testnode.isNull())
        return true;
    }
    case ID_SELECT: {
      DOM::HTMLSelectElement select = element;
      bool ok;
      uint u = propertyName.toULong(&ok);
      if (ok && !(select.options().item(u).isNull()))
        return true;
    }
    default:
      break;
  }

  return DOMElement::hasProperty(exec, propertyName);
}

UString KJS::HTMLElement::toString(ExecState *exec) const
{
  if (node.elementId() == ID_A)
    return UString(static_cast<const DOM::HTMLAnchorElement&>(node).href());
  else
    return DOMElement::toString(exec);
}

List KJS::HTMLElement::eventHandlerScope(ExecState *exec) const
{
  DOM::HTMLElement element = static_cast<DOM::HTMLElement>(node);

  List scope;
  // The element is the first one, so that it is the most prioritary
  scope.append(getDOMNode(exec,element));

  DOM::Node form = element.parentNode();
  while (!form.isNull() && form.elementId() != ID_FORM)
    form = form.parentNode();
  if (!form.isNull())
    scope.append(getDOMNode(exec,form));

  // The document is the last one, so that it is the least prioritary
  scope.append(getDOMNode(exec,element.ownerDocument()));
  return scope;
}

HTMLElementFunction::HTMLElementFunction(ExecState *exec, int i, int len)
  : DOMFunction(), id(i)
{
  Value protect(this);
  put(exec,"length",Number(len),DontDelete|ReadOnly|DontEnum);
}

Value KJS::HTMLElementFunction::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  if (!thisObj.inherits(&KJS::HTMLElement::info)) {
    Object err = Error::create(exec,TypeError);
    exec->setException(err);
    return err;
  }
  kdDebug() << "KJS::HTMLElementFunction::tryCall " << endl;
  DOM::HTMLElement element = static_cast<KJS::HTMLElement *>(thisObj.imp())->toElement();

  switch (element.elementId()) {
    case ID_FORM: {
      DOM::HTMLFormElement form = element;
      if (id == KJS::HTMLElement::FormSubmit) {
        form.submit();
        return Undefined();
      }
      else if (id == KJS::HTMLElement::FormReset) {
        form.reset();
        return Undefined();
      }
    }
    break;
    case ID_SELECT: {
      DOM::HTMLSelectElement select = element;
      if (id == KJS::HTMLElement::SelectAdd) {
        select.add(KJS::toNode(args[0]),KJS::toNode(args[1]));
        return Undefined();
      }
      else if (id == KJS::HTMLElement::SelectRemove) {
        select.remove(int(args[0].toNumber(exec)));
        return Undefined();
      }
      else if (id == KJS::HTMLElement::SelectBlur) {
        select.blur();
        return Undefined();
      }
      else if (id == KJS::HTMLElement::SelectFocus) {
        select.focus();
        return Undefined();
      }
    }
    break;
    case ID_INPUT: {
      DOM::HTMLInputElement input = element;
      if (id == KJS::HTMLElement::InputBlur) {
        input.blur();
        return Undefined();
      }
      else if (id == KJS::HTMLElement::InputFocus) {
        input.focus();
        return Undefined();
      }
      else if (id == KJS::HTMLElement::InputSelect) {
        input.select();
        return Undefined();
      }
      else if (id == KJS::HTMLElement::InputClick) {
        input.click();
        return Undefined();
      }
    }
    break;
    case ID_TEXTAREA: {
      DOM::HTMLTextAreaElement textarea = element;
      if (id == KJS::HTMLElement::TextAreaBlur) {
        textarea.blur();
        return Undefined();
      }
      else if (id == KJS::HTMLElement::TextAreaFocus) {
        textarea.focus();
        return Undefined();
      }
      else if (id == KJS::HTMLElement::TextAreaSelect) {
        textarea.select();
        return Undefined();
      }
    }
    break;
    case ID_A: {
      DOM::HTMLAnchorElement anchor = element;
      if (id == KJS::HTMLElement::AnchorBlur) {
        anchor.blur();
        return Undefined();
      }
      else if (id == KJS::HTMLElement::AnchorFocus) {
        anchor.focus();
        return Undefined();
      }
    }
    break;
    case ID_TABLE: {
      DOM::HTMLTableElement table = element;
      if (id == KJS::HTMLElement::TableCreateTHead)
        return getDOMNode(exec,table.createTHead());
      else if (id == KJS::HTMLElement::TableDeleteTHead) {
        table.deleteTHead();
        return Undefined();
      }
      else if (id == KJS::HTMLElement::TableCreateTFoot)
        return getDOMNode(exec,table.createTFoot());
      else if (id == KJS::HTMLElement::TableDeleteTFoot) {
        table.deleteTFoot();
        return Undefined();
      }
      else if (id == KJS::HTMLElement::TableCreateCaption)
        return getDOMNode(exec,table.createCaption());
      else if (id == KJS::HTMLElement::TableDeleteCaption) {
        table.deleteCaption();
        return Undefined();
      }
      else if (id == KJS::HTMLElement::TableInsertRow)
        return getDOMNode(exec,table.insertRow(args[0].toInteger(exec)));
      else if (id == KJS::HTMLElement::TableDeleteRow) {
        table.deleteRow(args[0].toInteger(exec));
        return Undefined();
      }
    }
    break;
    case ID_THEAD:
    case ID_TBODY:
    case ID_TFOOT: {
      DOM::HTMLTableSectionElement tableSection = element;
      if (id == KJS::HTMLElement::TableSectionInsertRow)
        return getDOMNode(exec,tableSection.insertRow(args[0].toInteger(exec)));
      else if (id == KJS::HTMLElement::TableSectionDeleteRow) {
        tableSection.deleteRow(args[0].toInteger(exec));
        return Undefined();
      }
    }
    break;
    case ID_TR: {
      DOM::HTMLTableRowElement tableRow = element;
      if (id == KJS::HTMLElement::TableRowInsertCell)
        return getDOMNode(exec,tableRow.insertCell(args[0].toInteger(exec)));
      else if (id == KJS::HTMLElement::TableRowDeleteCell) {
        tableRow.deleteCell(args[0].toInteger(exec));
        return Undefined();
      }
    }
    break;
  }

  return Undefined();
}

void KJS::HTMLElement::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
#ifdef KJS_VERBOSE
  DOM::DOMString str = value.isA(NullType) ? DOM::DOMString() : value.toString(exec).string();
#endif
  DOM::HTMLElement element = static_cast<DOM::HTMLElement>(node);
#ifdef KJS_VERBOSE
  kdDebug(6070) << "KJS::HTMLElement::tryPut " << propertyName.qstring()
                << " thisTag=" << element.tagName().string()
                << " str=" << str.string() << endl;
#endif
  // First look at dynamic properties
  switch (element.elementId()) {
    case ID_SELECT: {
      DOM::HTMLSelectElement select = element;
      bool ok;
      /*uint u =*/ propertyName.toULong(&ok);
      if (ok) {
        Object coll = Object::dynamicCast( getSelectHTMLCollection(exec, select.options(), select) );
        if ( !coll.isNull() )
          coll.put(exec,propertyName,value);
        return;
      }
    }
    break;
  default:
      break;
  }

  const HashTable* table = classInfo()->propHashTable; // get the right hashtable
  const HashEntry* entry = Lookup::findEntry(table, propertyName);
  if (entry) {
    if (entry->attr & Function) // function: put as override property
    {
      ObjectImp::put(exec, propertyName, value, attr);
      return;
    }
    else if ((entry->attr & ReadOnly) == 0) // let DOMObjectLookupPut print the warning if not
    {
      putValue(exec, entry->value, value, attr);
      return;
    }
  }
  DOMObjectLookupPut<KJS::HTMLElement, DOMElement>(exec, propertyName, value, attr, &KJS::HTMLElementTable, this);
}

void KJS::HTMLElement::putValue(ExecState *exec, int token, const Value& value, int)
{
  DOM::DOMString str = value.isA(NullType) ? DOM::DOMString() : value.toString(exec).string();
  DOM::Node n = (new DOMNode(exec, KJS::toNode(value)))->toNode();
  DOM::HTMLElement element = static_cast<DOM::HTMLElement>(node);
#ifdef KJS_VERBOSE
  kdDebug(6070) << "KJS::HTMLElement::putValue "
                << " thisTag=" << element.tagName().string()
                << " token=" << token << endl;
#endif

  switch (element.elementId()) {
  case ID_HTML: {
      DOM::HTMLHtmlElement html = element;
      switch (token) {
      case HtmlVersion:         { html.setVersion(str); return; }
      }
  }
  break;
  case ID_HEAD: {
    DOM::HTMLHeadElement head = element;
    switch (token) {
    case HeadProfile:         { head.setProfile(str); return; }
    }
  }
  break;
  case ID_LINK: {
    DOM::HTMLLinkElement link = element;
    switch (token) {
      case LinkDisabled:        { link.setDisabled(value.toBoolean(exec)); return; }
      case LinkCharset:         { link.setCharset(str); return; }
      case LinkHref:            { link.setHref(str); return; }
      case LinkHrefLang:        { link.setHreflang(str); return; }
      case LinkMedia:           { link.setMedia(str); return; }
      case LinkRel:             { link.setRel(str); return; }
      case LinkRev:             { link.setRev(str); return; }
      case LinkTarget:          { link.setTarget(str); return; }
      case LinkType:            { link.setType(str); return; }
      }
    }
    break;
    case ID_TITLE: {
      DOM::HTMLTitleElement title = element;
      switch (token) {
      case TitleText:                 { title.setText(str); return; }
      }
    }
    break;
    case ID_META: {
      DOM::HTMLMetaElement meta = element;
      switch (token) {
      case MetaContent:         { meta.setContent(str); return; }
      case MetaHttpEquiv:       { meta.setHttpEquiv(str); return; }
      case MetaName:            { meta.setName(str); return; }
      case MetaScheme:          { meta.setScheme(str); return; }
      }
    }
    break;
    case ID_BASE: {
      DOM::HTMLBaseElement base = element;
      switch (token) {
      case BaseHref:            { base.setHref(str); return; }
      case BaseTarget:          { base.setTarget(str); return; }
      }
    }
    break;
    case ID_ISINDEX: {
      DOM::HTMLIsIndexElement isindex = element;
      switch (token) {
      // read-only: form
      case IsIndexPrompt:               { isindex.setPrompt(str); return; }
      }
    }
    break;
    case ID_STYLE: {
      DOM::HTMLStyleElement style = element;
      switch (token) {
      case StyleDisabled:        { style.setDisabled(value.toBoolean(exec)); return; }
      case StyleMedia:           { style.setMedia(str); return; }
      case StyleType:            { style.setType(str); return; }
      }
    }
    break;
    case ID_BODY: {
      DOM::HTMLBodyElement body = element;
      switch (token) {
      case BodyALink:           { body.setALink(str); return; }
      case BodyBackground:      { body.setBackground(str); return; }
      case BodyBgColor:         { body.setBgColor(str); return; }
      case BodyLink:            { body.setLink(str); return; }
      case BodyText:            { body.setText(str); return; }
      case BodyVLink:           { body.setVLink(str); return; }
      }
    }
    break;
    case ID_FORM: {
      DOM::HTMLFormElement form = element;
      switch (token) {
      // read-only: elements
      // read-only: length
      case FormName:            { form.setName(str); return; }
      case FormAcceptCharset:   { form.setAcceptCharset(str); return; }
      case FormAction:          { form.setAction(str.string()); return; }
      case FormEncType:         { form.setEnctype(str); return; }
      case FormMethod:          { form.setMethod(str); return; }
      case FormTarget:          { form.setTarget(str); return; }
      }
    }
    break;
    case ID_SELECT: {
      DOM::HTMLSelectElement select = element;
      switch (token) {
      // read-only: type
      case SelectSelectedIndex:   { select.setSelectedIndex(value.toInteger(exec)); return; }
      case SelectValue:           { select.setValue(str); return; }
      case SelectLength:          { // read-only according to the NS spec, but webpages need it writeable
                                         Object coll = Object::dynamicCast( getSelectHTMLCollection(exec, select.options(), select) );
                                         if ( !coll.isNull() )
                                           coll.put(exec,"length",value);
                                         return;
                                       }
      // read-only: form
      // read-only: options
      case SelectDisabled:        { select.setDisabled(value.toBoolean(exec)); return; }
      case SelectMultiple:        { select.setMultiple(value.toBoolean(exec)); return; }
      case SelectName:            { select.setName(str); return; }
      case SelectSize:            { select.setSize(value.toInteger(exec)); return; }
      case SelectTabIndex:        { select.setTabIndex(value.toInteger(exec)); return; }
      }
    }
    break;
    case ID_OPTGROUP: {
      DOM::HTMLOptGroupElement optgroup = element;
      switch (token) {
      case OptGroupDisabled:        { optgroup.setDisabled(value.toBoolean(exec)); return; }
      case OptGroupLabel:           { optgroup.setLabel(str); return; }
      }
    }
    break;
    case ID_OPTION: {
      DOM::HTMLOptionElement option = element;
      switch (token) {
      // read-only: form
      case OptionDefaultSelected: { option.setDefaultSelected(value.toBoolean(exec)); return; }
      // read-only: text  <--- According to the DOM, but JavaScript and JScript both allow changes.
      // So, we'll do it here and not add it to our DOM headers.
      case OptionText:            { DOM::NodeList nl(option.childNodes());
                                    for (unsigned int i = 0; i < nl.length(); i++) {
                                        if (nl.item(i).nodeType() == DOM::Node::TEXT_NODE) {
                                            static_cast<DOM::Text>(nl.item(i)).setData(str);
                                            return;
                                        }
                                  }
                                  // No child text node found, creating one
                                  DOM::Text t = option.ownerDocument().createTextNode(str);
                                  try { option.appendChild(t); }
                                  catch(DOM::DOMException& e) {
                                    // #### exec->setException ?
                                  }

                                  return;
      }
      // read-only: index
      case OptionDisabled:        { option.setDisabled(value.toBoolean(exec)); return; }
      case OptionLabel:           { option.setLabel(str); return; }
      case OptionSelected:        { option.setSelected(value.toBoolean(exec)); return; }
      case OptionValue:           { option.setValue(str); return; }
      }
    }
    break;
    case ID_INPUT: {
      DOM::HTMLInputElement input = element;
      switch (token) {
      case InputDefaultValue:    { input.setDefaultValue(str); return; }
      case InputDefaultChecked:  { input.setDefaultChecked(value.toBoolean(exec)); return; }
      // read-only: form
      case InputAccept:          { input.setAccept(str); return; }
      case InputAccessKey:       { input.setAccessKey(str); return; }
      case InputAlign:           { input.setAlign(str); return; }
      case InputAlt:             { input.setAlt(str); return; }
      case InputChecked:         { input.setChecked(value.toBoolean(exec)); return; }
      case InputDisabled:        { input.setDisabled(value.toBoolean(exec)); return; }
      case InputMaxLength:       { input.setMaxLength(value.toInteger(exec)); return; }
      case InputName:            { input.setName(str); return; }
      case InputReadOnly:        { input.setReadOnly(value.toBoolean(exec)); return; }
      case InputSize:            { input.setSize(str); return; }
      case InputSrc:             { input.setSrc(str); return; }
      case InputTabIndex:        { input.setTabIndex(value.toInteger(exec)); return; }
      case InputType:            { input.setType(str); return; }
      case InputUseMap:          { input.setUseMap(str); return; }
      case InputValue:           { input.setValue(str); return; }
      }
    }
    break;
    case ID_TEXTAREA: {
      DOM::HTMLTextAreaElement textarea = element;
      switch (token) {
      case TextAreaDefaultValue:    { textarea.setDefaultValue(str); return; }
      // read-only: form
      case TextAreaAccessKey:       { textarea.setAccessKey(str); return; }
      case TextAreaCols:            { textarea.setCols(value.toInteger(exec)); return; }
      case TextAreaDisabled:        { textarea.setDisabled(value.toBoolean(exec)); return; }
      case TextAreaName:            { textarea.setName(str); return; }
      case TextAreaReadOnly:        { textarea.setReadOnly(value.toBoolean(exec)); return; }
      case TextAreaRows:            { textarea.setRows(value.toInteger(exec)); return; }
      case TextAreaTabIndex:        { textarea.setTabIndex(value.toInteger(exec)); return; }
      // read-only: type
      case TextAreaValue:           { textarea.setValue(str); return; }
      }
    }
    break;
    case ID_BUTTON: {
      DOM::HTMLButtonElement button = element;
      switch (token) {
      // read-only: form
      case ButtonAccessKey:       { button.setAccessKey(str); return; }
      case ButtonDisabled:        { button.setDisabled(value.toBoolean(exec)); return; }
      case ButtonName:            { button.setName(str); return; }
      case ButtonTabIndex:        { button.setTabIndex(value.toInteger(exec)); return; }
      // read-only: type
      case ButtonValue:           { button.setValue(str); return; }
      }
    }
    break;
    case ID_LABEL: {
      DOM::HTMLLabelElement label = element;
      switch (token) {
      // read-only: form
      case LabelAccessKey:       { label.setAccessKey(str); return; }
      case LabelHtmlFor:         { label.setHtmlFor(str); return; }
      }
    }
    break;
//    case ID_FIELDSET: {
//      DOM::HTMLFieldSetElement fieldSet = element;
//      // read-only: form
//    }
//    break;
    case ID_LEGEND: {
      DOM::HTMLLegendElement legend = element;
      switch (token) {
      // read-only: form
      case LegendAccessKey:       { legend.setAccessKey(str); return; }
      case LegendAlign:           { legend.setAlign(str); return; }
      }
    }
    break;
    case ID_UL: {
      DOM::HTMLUListElement uList = element;
      switch (token) {
      case UListCompact:         { uList.setCompact(value.toBoolean(exec)); return; }
      case UListType:            { uList.setType(str); return; }
      }
    }
    break;
    case ID_OL: {
      DOM::HTMLOListElement oList = element;
      switch (token) {
      case OListCompact:         { oList.setCompact(value.toBoolean(exec)); return; }
      case OListStart:           { oList.setStart(value.toInteger(exec)); return; }
      case OListType:            { oList.setType(str); return; }
      }
    }
    break;
    case ID_DL: {
      DOM::HTMLDListElement dList = element;
      switch (token) {
      case DListCompact:         { dList.setCompact(value.toBoolean(exec)); return; }
      }
    }
    break;
    case ID_DIR: {
      DOM::HTMLDirectoryElement directory = element;
      switch (token) {
      case DirectoryCompact:     { directory.setCompact(value.toBoolean(exec)); return; }
      }
    }
    break;
    case ID_MENU: {
      DOM::HTMLMenuElement menu = element;
      switch (token) {
      case MenuCompact:         { menu.setCompact(value.toBoolean(exec)); return; }
      }
    }
    break;
    case ID_LI: {
      DOM::HTMLLIElement li = element;
      switch (token) {
      case LIType:            { li.setType(str); return; }
      case LIValue:           { li.setValue(value.toInteger(exec)); return; }
      }
    }
    break;
    case ID_DIV: {
      DOM::HTMLDivElement div = element;
      switch (token) {
      case DivAlign:           { div.setAlign(str); return; }
      }
    }
    break;
    case ID_P: {
      DOM::HTMLParagraphElement paragraph = element;
      switch (token) {
      case ParagraphAlign:     { paragraph.setAlign(str); return; }
      }
    }
    break;
    case ID_H1:
    case ID_H2:
    case ID_H3:
    case ID_H4:
    case ID_H5:
    case ID_H6: {
      DOM::HTMLHeadingElement heading = element;
      switch (token) {
      case HeadingAlign:         { heading.setAlign(str); return; }
      }
    }
    break;
    case ID_BLOCKQUOTE: {
      DOM::HTMLBlockquoteElement blockquote = element;
      switch (token) {
      case BlockQuoteCite:       { blockquote.setCite(str); return; }
      }
    }
    break;
    case ID_Q: {
      DOM::HTMLQuoteElement quote = element;
      switch (token) {
      case QuoteCite:            { quote.setCite(str); return; }
      }
    }
    break;
    case ID_PRE: {
      DOM::HTMLPreElement pre = element;
      switch (token) {
      case PreWidth:           { pre.setWidth(value.toInteger(exec)); return; }
      }
    }
    break;
    case ID_BR: {
      DOM::HTMLBRElement br = element;
      switch (token) {
      case BRClear:           { br.setClear(str); return; }
      }
    }
    break;
    case ID_BASEFONT: {
      DOM::HTMLBaseFontElement baseFont = element;
      switch (token) {
      case BaseFontColor:           { baseFont.setColor(str); return; }
      case BaseFontFace:            { baseFont.setFace(str); return; }
      case BaseFontSize:            { baseFont.setSize(str); return; }
      }
    }
    break;
    case ID_FONT: {
      DOM::HTMLFontElement font = element;
      switch (token) {
      case FontColor:           { font.setColor(str); return; }
      case FontFace:            { font.setFace(str); return; }
      case FontSize:            { font.setSize(str); return; }
      }
    }
    break;
    case ID_HR: {
      DOM::HTMLHRElement hr = element;
      switch (token) {
      case HRAlign:           { hr.setAlign(str); return; }
      case HRNoShade:         { hr.setNoShade(value.toBoolean(exec)); return; }
      case HRSize:            { hr.setSize(str); return; }
      case HRWidth:           { hr.setWidth(str); return; }
      }
    }
    break;
    case ID_INS:
    case ID_DEL: {
      DOM::HTMLModElement mod = element;
      switch (token) {
      case ModCite:            { mod.setCite(str); return; }
      case ModDateTime:        { mod.setDateTime(str); return; }
      }
    }
    break;
    case ID_A: {
      DOM::HTMLAnchorElement anchor = element;
      switch (token) {
      case AnchorAccessKey:       { anchor.setAccessKey(str); return; }
      case AnchorCharset:         { anchor.setCharset(str); return; }
      case AnchorCoords:          { anchor.setCoords(str); return; }
      case AnchorHref:            { anchor.setHref(str); return; }
      case AnchorHrefLang:        { anchor.setHreflang(str); return; }
      case AnchorName:            { anchor.setName(str); return; }
      case AnchorRel:             { anchor.setRel(str); return; }
      case AnchorRev:             { anchor.setRev(str); return; }
      case AnchorShape:           { anchor.setShape(str); return; }
      case AnchorTabIndex:        { anchor.setTabIndex(value.toInteger(exec)); return; }
      case AnchorTarget:          { anchor.setTarget(str); return; }
      case AnchorType:            { anchor.setType(str); return; }
      }
    }
    break;
    case ID_IMG: {
      DOM::HTMLImageElement image = element;
      switch (token) {
      case ImageName:            { image.setName(str); return; }
      case ImageAlign:           { image.setAlign(str); return; }
      case ImageAlt:             { image.setAlt(str); return; }
      case ImageBorder:          { image.setBorder(value.toInteger(exec)); return; }
      case ImageHeight:          { image.setHeight(value.toInteger(exec)); return; }
      case ImageHspace:          { image.setHspace(value.toInteger(exec)); return; }
      case ImageIsMap:           { image.setIsMap(value.toBoolean(exec)); return; }
      case ImageLongDesc:        { image.setLongDesc(str); return; }
      case ImageSrc:             { image.setSrc(str); return; }
      case ImageUseMap:          { image.setUseMap(str); return; }
      case ImageVspace:          { image.setVspace(value.toInteger(exec)); return; }
      case ImageWidth:           { image.setWidth(value.toInteger(exec)); return; }
      }
    }
    break;
    case ID_OBJECT: {
      DOM::HTMLObjectElement object = element;
      switch (token) {
      // read-only: form
      case ObjectCode:                 { object.setCode(str); return; }
      case ObjectAlign:           { object.setAlign(str); return; }
      case ObjectArchive:         { object.setArchive(str); return; }
      case ObjectBorder:          { object.setBorder(str); return; }
      case ObjectCodeBase:        { object.setCodeBase(str); return; }
      case ObjectCodeType:        { object.setCodeType(str); return; }
      // read-only: ObjectContentDocument
      case ObjectData:            { object.setData(str); return; }
      case ObjectDeclare:         { object.setDeclare(value.toBoolean(exec)); return; }
      case ObjectHeight:          { object.setHeight(str); return; }
      case ObjectHspace:          { object.setHspace(str); return; }
      case ObjectName:            { object.setName(str); return; }
      case ObjectStandby:         { object.setStandby(str); return; }
      case ObjectTabIndex:        { object.setTabIndex(value.toInteger(exec)); return; }
      case ObjectType:            { object.setType(str); return; }
      case ObjectUseMap:          { object.setUseMap(str); return; }
      case ObjectVspace:          { object.setVspace(str); return; }
      case ObjectWidth:           { object.setWidth(str); return; }
      }
    }
    break;
    case ID_PARAM: {
      DOM::HTMLParamElement param = element;
      switch (token) {
      case ParamName:            { param.setName(str); return; }
      case ParamType:            { param.setType(str); return; }
      case ParamValue:           { param.setValue(str); return; }
      case ParamValueType:       { param.setValueType(str); return; }
      }
    }
    break;
    case ID_APPLET: {
      DOM::HTMLAppletElement applet = element;
      switch (token) {
      case AppletAlign:           { applet.setAlign(str); return; }
      case AppletAlt:             { applet.setAlt(str); return; }
      case AppletArchive:         { applet.setArchive(str); return; }
      case AppletCode:            { applet.setCode(str); return; }
      case AppletCodeBase:        { applet.setCodeBase(str); return; }
      case AppletHeight:          { applet.setHeight(str); return; }
      case AppletHspace:          { applet.setHspace(str); return; }
      case AppletName:            { applet.setName(str); return; }
      case AppletObject:          { applet.setObject(str); return; }
      case AppletVspace:          { applet.setVspace(str); return; }
      case AppletWidth:           { applet.setWidth(str); return; }
      }
    }
    break;
    case ID_MAP: {
      DOM::HTMLMapElement map = element;
      switch (token) {
      // read-only: areas
      case MapName:                 { map.setName(str); return; }
     }
    }
    break;
    case ID_AREA: {
      DOM::HTMLAreaElement area = element;
      switch (token) {
      case AreaAccessKey:       { area.setAccessKey(str); return; }
      case AreaAlt:             { area.setAlt(str); return; }
      case AreaCoords:          { area.setCoords(str); return; }
      case AreaHref:            { area.setHref(str); return; }
      case AreaNoHref:          { area.setNoHref(value.toBoolean(exec)); return; }
      case AreaShape:           { area.setShape(str); return; }
      case AreaTabIndex:        { area.setTabIndex(value.toInteger(exec)); return; }
      case AreaTarget:          { area.setTarget(str); return; }
      }
    }
    break;
    case ID_SCRIPT: {
      DOM::HTMLScriptElement script = element;
      switch (token) {
      case ScriptText:            { script.setText(str); return; }
      case ScriptHtmlFor:         { script.setHtmlFor(str); return; }
      case ScriptEvent:           { script.setEvent(str); return; }
      case ScriptCharset:         { script.setCharset(str); return; }
      case ScriptDefer:           { script.setDefer(value.toBoolean(exec)); return; }
      case ScriptSrc:             { script.setSrc(str); return; }
      case ScriptType:            { script.setType(str); return; }
      }
    }
    break;
    case ID_TABLE: {
      DOM::HTMLTableElement table = element;
      switch (token) {
      case TableCaption:         { table.setCaption(n); return; } // type HTMLTableCaptionElement
      case TableTHead:           { table.setTHead(n); return; } // type HTMLTableSectionElement
      case TableTFoot:           { table.setTFoot(n); return; } // type HTMLTableSectionElement
      // read-only: rows
      // read-only: tbodies
      case TableAlign:           { table.setAlign(str); return; }
      case TableBgColor:         { table.setBgColor(str); return; }
      case TableBorder:          { table.setBorder(str); return; }
      case TableCellPadding:     { table.setCellPadding(str); return; }
      case TableCellSpacing:     { table.setCellSpacing(str); return; }
      case TableFrame:           { table.setFrame(str); return; }
      case TableRules:           { table.setRules(str); return; }
      case TableSummary:         { table.setSummary(str); return; }
      case TableWidth:           { table.setWidth(str); return; }
      }
    }
    break;
    case ID_CAPTION: {
      DOM::HTMLTableCaptionElement tableCaption;
      switch (token) {
      case TableAlign:           { tableCaption.setAlign(str); return; }
      }
    }
    break;
    case ID_COL: {
      DOM::HTMLTableColElement tableCol = element;
      switch (token) {
      case TableColAlign:           { tableCol.setAlign(str); return; }
      case TableColCh:              { tableCol.setCh(str); return; }
      case TableColChOff:           { tableCol.setChOff(str); return; }
      case TableColSpan:            { tableCol.setSpan(value.toInteger(exec)); return; }
      case TableColVAlign:          { tableCol.setVAlign(str); return; }
      case TableColWidth:           { tableCol.setWidth(str); return; }
      }
    }
    break;
    case ID_THEAD:
    case ID_TBODY:
    case ID_TFOOT: {
      DOM::HTMLTableSectionElement tableSection = element;
      switch (token) {
      case TableSectionAlign:           { tableSection.setAlign(str); return; }
      case TableSectionCh:              { tableSection.setCh(str); return; }
      case TableSectionChOff:           { tableSection.setChOff(str); return; }
      case TableSectionVAlign:          { tableSection.setVAlign(str); return; }
      // read-only: rows
      }
    }
    break;
    case ID_TR: {
      DOM::HTMLTableRowElement tableRow = element;
      switch (token) {
      // read-only: rowIndex
      // read-only: sectionRowIndex
      // read-only: cells
      case TableRowAlign:           { tableRow.setAlign(str); return; }
      case TableRowBgColor:         { tableRow.setBgColor(str); return; }
      case TableRowCh:              { tableRow.setCh(str); return; }
      case TableRowChOff:           { tableRow.setChOff(str); return; }
      case TableRowVAlign:          { tableRow.setVAlign(str); return; }
      }
    }
    break;
    case ID_TH:
    case ID_TD: {
      DOM::HTMLTableCellElement tableCell = element;
      switch (token) {
      // read-only: cellIndex
      case TableCellAbbr:            { tableCell.setAbbr(str); return; }
      case TableCellAlign:           { tableCell.setAlign(str); return; }
      case TableCellAxis:            { tableCell.setAxis(str); return; }
      case TableCellBgColor:         { tableCell.setBgColor(str); return; }
      case TableCellCh:              { tableCell.setCh(str); return; }
      case TableCellChOff:           { tableCell.setChOff(str); return; }
      case TableCellColSpan:         { tableCell.setColSpan(value.toInteger(exec)); return; }
      case TableCellHeaders:         { tableCell.setHeaders(str); return; }
      case TableCellHeight:          { tableCell.setHeight(str); return; }
      case TableCellNoWrap:          { tableCell.setNoWrap(value.toBoolean(exec)); return; }
      case TableCellRowSpan:         { tableCell.setRowSpan(value.toInteger(exec)); return; }
      case TableCellScope:           { tableCell.setScope(str); return; }
      case TableCellVAlign:          { tableCell.setVAlign(str); return; }
      case TableCellWidth:           { tableCell.setWidth(str); return; }
      }
    }
    break;
    case ID_FRAMESET: {
      DOM::HTMLFrameSetElement frameSet = element;
      switch (token) {
      case FrameSetCols:            { frameSet.setCols(str); return; }
      case FrameSetRows:            { frameSet.setRows(str); return; }
      }
    }
    break;
    case ID_FRAME: {
      DOM::HTMLFrameElement frameElement = element;
      switch (token) {
       // read-only: FrameContentDocument:
      case FrameFrameBorder:     { frameElement.setFrameBorder(str); return; }
      case FrameLongDesc:        { frameElement.setLongDesc(str); return; }
      case FrameMarginHeight:    { frameElement.setMarginHeight(str); return; }
      case FrameMarginWidth:     { frameElement.setMarginWidth(str); return; }
      case FrameName:            { frameElement.setName(str); return; }
      case FrameNoResize:        { frameElement.setNoResize(value.toBoolean(exec)); return; }
      case FrameScrolling:       { frameElement.setScrolling(str); return; }
      case FrameSrc:             { frameElement.setSrc(str); return; }
      case FrameLocation:        {
                                   static_cast<DOM::HTMLFrameElementImpl *>(frameElement.handle())->setLocation(str);
                                   return;
                                 }
      }
    }
    break;
    case ID_IFRAME: {
      DOM::HTMLIFrameElement iFrame = element;
      switch (token) {
      case IFrameAlign:           { iFrame.setAlign(str); return; }
      // read-only: IFrameContentDocument
      case IFrameFrameBorder:     { iFrame.setFrameBorder(str); return; }
      case IFrameHeight:          { iFrame.setHeight(str); return; }
      case IFrameLongDesc:        { iFrame.setLongDesc(str); return; }
      case IFrameMarginHeight:    { iFrame.setMarginHeight(str); return; }
      case IFrameMarginWidth:     { iFrame.setMarginWidth(str); return; }
      case IFrameName:            { iFrame.setName(str); return; }
      case IFrameScrolling:       { iFrame.setScrolling(str); return; }
      case IFrameSrc:             { iFrame.setSrc(str); return; }
      case IFrameWidth:           { iFrame.setWidth(str); return; }
      }
      break;
    }
  }

  // generic properties
  switch (token) {
  case ElementId:
    element.setId(str);
    return;
  case ElementTitle:
    element.setTitle(str);
    return;
  case ElementLang:
    element.setLang(str);
    return;
  case ElementDir:
    element.setDir(str);
    return;
  case ElementClassName:
    element.setClassName(str);
    return;
  case ElementInnerHTML:
    element.setInnerHTML(str);
    return;
  case ElementInnerText:
    element.setInnerText(str);
    return;
  default:
    kdWarning() << "KJS::HTMLElement::putValue unhandled token " << token << " thisTag=" << element.tagName().string() << " str=" << str.string() << endl;
  }
}

// -------------------------------------------------------------------------
/* Source for HTMLCollectionProtoTable. Use "make hashtables" to regenerate.
@begin HTMLCollectionProtoTable 3
  item		HTMLCollection::Item		DontDelete|Function 1
  namedItem	HTMLCollection::NamedItem	DontDelete|Function 1
  tags		HTMLCollection::Tags		DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("HTMLCollection", HTMLCollectionProto)
IMPLEMENT_PROTOFUNC(HTMLCollectionProtoFunc)
IMPLEMENT_PROTOTYPE(HTMLCollectionProto,HTMLCollectionProtoFunc)

const ClassInfo HTMLCollection::info = { "HTMLCollection", 0, 0, 0 };

HTMLCollection::HTMLCollection(ExecState *exec, DOM::HTMLCollection c)
  : DOMObject(HTMLCollectionProto::self(exec)), collection(c) {}

HTMLCollection::~HTMLCollection()
{
  ScriptInterpreter::forgetDOMObject(collection.handle());
}

// We have to implement hasProperty since we don't use a hashtable for 'selectedIndex' and 'length'
// ## this breaks "for (..in..)" though.
bool KJS::HTMLCollection::hasProperty(ExecState *exec, const UString &p) const
{
  if (p == "selectedIndex" || p == "length")
    return true;
  return DOMObject::hasProperty(exec, p);
}

Value KJS::HTMLCollection::tryGet(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug() << "KJS::HTMLCollection::tryGet " << propertyName.ascii() << endl;
#endif
  if (propertyName == "length")
    return Number(collection.length());
  else if (propertyName == "selectedIndex" &&
	   collection.item(0).elementId() == ID_OPTION) {
    // NON-STANDARD options.selectedIndex
    DOM::Node node = collection.item(0).parentNode();
    while(!node.isNull()) {
      if(node.elementId() == ID_SELECT) {
	DOM::HTMLSelectElement sel = static_cast<DOM::HTMLSelectElement>(node);
	return Number(sel.selectedIndex());
      }
      node = node.parentNode();
    }
    return Undefined();
  } else {
    // Look in the prototype (for functions) before assuming it's an item's name
    Object proto = Object::dynamicCast(prototype());
    if (!proto.isNull() && proto.hasProperty(exec,propertyName))
      return proto.get(exec,propertyName);

    // name or index ?
    bool ok;
    unsigned int u = propertyName.toULong(&ok);
    if (ok) {
      DOM::Node node = collection.item(u);
      return getDOMNode(exec,node);
    }
    else
      return getNamedItems(exec,propertyName);
  }
}

// HTMLCollections are strange objects, they support both get and call,
// so that document.forms.item(0) and document.forms(0) both work.
Value KJS::HTMLCollection::call(ExecState *exec, Object &thisObj, const List &args)
{
  // This code duplication is necessary, HTMLCollection isn't a DOMFunction
  Value val;
  try {
    val = tryCall(exec, thisObj, args);
  }
  // pity there's no way to distinguish between these in JS code
  catch (...) {
    Object err = Error::create(exec, GeneralError, "Exception from HTMLCollection");
    exec->setException(err);
  }
  return val;
}

Value KJS::HTMLCollection::tryCall(ExecState *exec, Object &, const List &args)
{
  // Do not use thisObj here. It can be the HTMLDocument, in the document.forms(i) case.
  /*if( thisObj.imp() != this )
  {
    kdWarning() << "thisObj.imp() != this in HTMLCollection::tryCall" << endl;
    KJS::printInfo(exec,"KJS::HTMLCollection::tryCall thisObj",thisObj,-1);
    KJS::printInfo(exec,"KJS::HTMLCollection::tryCall this",Value(this),-1);
  }*/
  // Also, do we need the TypeError test here ?

  if (args.size() == 1) {
    // support for document.all(<index>) etc.
    bool ok;
    UString s = args[0].toString(exec);
    unsigned int u = s.toULong(&ok);
    if (ok) {
      DOM::Element element = collection.item(u);
      return getDOMNode(exec,element);
    }
    // support for document.images('<name>') etc.
    return getNamedItems(exec,s);
  }
  else if (args.size() >= 1) // the second arg, if set, is the index of the item we want
  {
    bool ok;
    UString s = args[0].toString(exec);
    unsigned int u = args[1].toString(exec).toULong(&ok);
    if (ok)
    {
      DOM::DOMString pstr = s.string();
      DOM::Node node = collection.namedItem(pstr);
      while (!node.isNull()) {
        if (!u)
          return getDOMNode(exec,node);
        node = collection.nextNamedItem(pstr);
        --u;
      }
    }
  }
  return Undefined();
}

Value KJS::HTMLCollection::getNamedItems(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "KJS::HTMLCollection::getNamedItems " << propertyName.ascii() << endl;
#endif
  DOM::DOMString pstr = propertyName.string();
  DOM::Node node = collection.namedItem(pstr);
  if(!node.isNull())
  {
    DOM::Node next = collection.nextNamedItem(pstr);
    if (next.isNull()) // single item
    {
#ifdef KJS_VERBOSE
      kdDebug(6070) << "returning single node" << endl;
#endif
      return getDOMNode(exec,node);
    }
    else // multiple items, return a collection
    {
      QValueList<DOM::Node> nodes;
      nodes.append(node);
      do {
        nodes.append(next);
        next = collection.nextNamedItem(pstr);
      } while (!next.isNull());
#ifdef KJS_VERBOSE
      kdDebug(6070) << "returning list of " << nodes.count() << " nodes" << endl;
#endif
      return Value(new DOMNamedNodesCollection(exec,nodes));
    }
  }
#ifdef KJS_VERBOSE
  kdDebug(6070) << "not found" << endl;
#endif
  return Undefined();
}

Value KJS::HTMLCollectionProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  if (!thisObj.inherits(&KJS::HTMLCollection::info)) {
    Object err = Error::create(exec,TypeError);
    exec->setException(err);
    return err;
  }
  DOM::HTMLCollection coll = static_cast<KJS::HTMLCollection *>(thisObj.imp())->toCollection();

  switch (id) {
  case KJS::HTMLCollection::Item:
    return getDOMNode(exec,coll.item(args[0].toUInt32(exec)));
  case KJS::HTMLCollection::Tags:
  {
    DOM::DOMString tagName = args[0].toString(exec).string();
    DOM::NodeList list;
    // getElementsByTagName exists in Document and in Element, pick up the right one
    if ( coll.base().nodeType() == DOM::Node::DOCUMENT_NODE )
    {
      DOM::Document doc = coll.base();
      list = doc.getElementsByTagName(tagName);
#ifdef KJS_VERBOSE
      kdDebug() << "KJS::HTMLCollectionProtoFunc::tryCall document.tags(" << tagName.string() << ") -> " << list.length() << " items in node list" << endl;
#endif
    } else
    {
      DOM::Element e = coll.base();
      list = e.getElementsByTagName(tagName);
#ifdef KJS_VERBOSE
      kdDebug() << "KJS::HTMLCollectionProtoFunc::tryCall element.tags(" << tagName.string() << ") -> " << list.length() << " items in node list" << endl;
#endif
    }
    return getDOMNodeList(exec, list);
  }
  case KJS::HTMLCollection::NamedItem:
    return static_cast<HTMLCollection *>(thisObj.imp())->getNamedItems(exec,args[0].toString(exec).string());
  default:
    return Undefined();
  }
}

Value KJS::HTMLSelectCollection::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "selectedIndex")
    return Number(element.selectedIndex());

  return  HTMLCollection::tryGet(exec, p);
}

void KJS::HTMLSelectCollection::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "KJS::HTMLSelectCollection::tryPut " << propertyName.qstring() << endl;
#endif
  if ( propertyName == "selectedIndex" ) {
    element.setSelectedIndex( value.toInteger( exec ) );
    return;
  }
  // resize ?
  else if (propertyName == "length") {
    long newLen = value.toInteger(exec);
    long diff = element.length() - newLen;

    if (diff < 0) { // add dummy elements
      do {
        element.add(element.ownerDocument().createElement("OPTION"), DOM::HTMLElement());
      } while (++diff);
    }
    else // remove elements
      while (diff-- > 0)
        element.remove(newLen);

    return;
  }
  // an index ?
  bool ok;
  unsigned int u = propertyName.toULong(&ok);
  if (!ok)
    return;

  if (value.isA(NullType) || value.isA(UndefinedType)) {
    // null and undefined delete. others, too ?
    element.remove(u);
    return;
  }

  // is v an option element ?
  DOM::Node node = KJS::toNode(value);
  if (node.isNull() || node.elementId() != ID_OPTION)
    return;

  DOM::HTMLOptionElement option = static_cast<DOM::HTMLOptionElement>(node);
  long diff = long(u) - element.length();
  DOM::HTMLElement before;
  // out of array bounds ? first insert empty dummies
  if (diff > 0) {
    while (diff--) {
      element.add(element.ownerDocument().createElement("OPTION"), before);
    }
    // replace an existing entry ?
  } else if (diff < 0) {
    before = element.options().item(u+1);
    element.remove(u);
  }
  // finally add the new element
  element.add(option, before);
}

////////////////////// Option Object ////////////////////////

OptionConstructorImp::OptionConstructorImp(ExecState *exec, const DOM::Document &d)
    : ObjectImp(), doc(d)
{
  // ## isn't there some redundancy between ObjectImp::_proto and the "prototype" property ?
  //put(exec,"prototype", ...,DontEnum|DontDelete|ReadOnly);

  // no. of arguments for constructor
  // ## is 4 correct ? 0 to 4, it seems to be
  put(exec,"length", Number(4), ReadOnly|DontDelete|DontEnum);
}

bool OptionConstructorImp::implementsConstruct() const
{
  return true;
}

Object OptionConstructorImp::construct(ExecState *exec, const List &args)
{
  DOM::Element el = doc.createElement("OPTION");
  DOM::HTMLOptionElement opt = static_cast<DOM::HTMLOptionElement>(el);
  int sz = args.size();
  DOM::Text t = doc.createTextNode("");
  try { opt.appendChild(t); }
  catch(DOM::DOMException& e) {
    // #### exec->setException ?
  }
  if (sz > 0)
    t.setData(args[0].toString(exec).string()); // set the text
  if (sz > 1)
    opt.setValue(args[1].toString(exec).string());
  if (sz > 2)
    opt.setDefaultSelected(args[2].toBoolean(exec));
  if (sz > 3)
    opt.setSelected(args[3].toBoolean(exec));

  return Object::dynamicCast(getDOMNode(exec,opt));
}

////////////////////// Image Object ////////////////////////

ImageConstructorImp::ImageConstructorImp(ExecState *, const DOM::Document &d)
    : ObjectImp(), doc(d)
{
}

bool ImageConstructorImp::implementsConstruct() const
{
  return true;
}

Object ImageConstructorImp::construct(ExecState *, const List &)
{
  /* TODO: fetch optional height & width from arguments */

  Object result(new Image(doc));
  /* TODO: do we need a prototype ? */

  return result;
}

const ClassInfo KJS::Image::info = { "Image", 0, &ImageTable, 0 };

/* Source for ImageTable. Use "make hashtables" to regenerate.
@begin ImageTable 3
  src		Image::Src		DontDelete
  complete	Image::Complete		DontDelete|ReadOnly
@end
*/

Value Image::tryGet(ExecState *exec, const UString &propertyName) const
{
  return DOMObjectLookupGetValue<Image,DOMObject>(exec, propertyName, &ImageTable, this);
}

Value Image::getValueProperty(ExecState *, int token) const
{
  switch (token) {
  case Src:
    return String(src);
  case Complete:
    return Boolean(!img || img->status() >= khtml::CachedObject::Persistent);
  default:
    kdWarning() << "Image::getValueProperty unhandled token " << token << endl;
    return Value();
  }
}

void Image::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
  // Not worth using the hashtable
  if (propertyName == "src") {
    String str = value.toString(exec);
    src = str.value();
    if ( img ) img->deref(this);
    img = static_cast<DOM::DocumentImpl*>( doc.handle() )->docLoader()->requestImage( src.string() );
    if ( img ) img->ref(this);
  } else {
    DOMObject::tryPut(exec, propertyName, value, attr);
  }
}

Image::~Image()
{
  if ( img ) img->deref(this);
}

Value KJS::getHTMLCollection(ExecState *exec,DOM::HTMLCollection c)
{
  return cacheDOMObject<DOM::HTMLCollection, KJS::HTMLCollection>(exec, c);
}

Value KJS::getSelectHTMLCollection(ExecState *exec, DOM::HTMLCollection c, DOM::HTMLSelectElement e)
{
  DOMObject *ret;
  if (c.isNull())
    return Null();
  ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->interpreter());
  if ((ret = interp->getDOMObject(c.handle())))
    return Value(ret);
  else {
    ret = new HTMLSelectCollection(exec, c, e);
    interp->putDOMObject(c.handle(),ret);
    return Value(ret);
  }
}
