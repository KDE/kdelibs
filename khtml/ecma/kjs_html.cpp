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

#include <khtml_part.h>
#include <misc/loader.h>
#include <dom/html_block.h>
#include <dom/html_head.h>
#include <dom/html_image.h>
#include <dom/html_inline.h>
#include <dom/html_list.h>
#include <dom/html_table.h>
#include <dom/html_object.h>
#include <dom_exception.h>

// ### HACK
#include <html/html_baseimpl.h>
#include <xml/dom2_eventsimpl.h>
#include <khtmlview.h>

#include "kjs_css.h"
#include "kjs_html.h"
#include "kjs_window.h"
#include "kjs_events.h"
#include "kjs_html.lut.h"

#include <htmltags.h>
#include <kdebug.h>

using namespace KJS;

QPtrDict<KJS::HTMLCollection> htmlCollections;

KJS::HTMLDocFunction::HTMLDocFunction(ExecState *exec, DOM::HTMLDocument d, int i, int len)
  : DOMFunction(), doc(d), id(i)
{
  Value protect(this);
  put(exec,"length",Number(len),DontDelete|ReadOnly|DontEnum);
}

Value KJS::HTMLDocFunction::tryGet(ExecState *exec, const UString &p) const
{
  // Support for document.images.length, .item, etc.
  DOM::HTMLCollection coll;

  switch (id) {
  case HTMLDocument::Images:
    coll = doc.images();
    break;
  case HTMLDocument::Applets:
    coll = doc.applets();
    break;
  case HTMLDocument::Links:
    coll = doc.links();
    break;
  case HTMLDocument::Forms:
    coll = doc.forms();
    break;
  case HTMLDocument::Anchors:
    coll = doc.anchors();
    break;
  case HTMLDocument::All:  // IE specific, not part of the DOM
    coll = doc.all();
    break;
  default:
    return Undefined();
  }

  Object tmp(new KJS::HTMLCollection(exec, coll));

  return tmp.get(exec, p);
}

Value KJS::HTMLDocFunction::tryCall(ExecState *exec, Object &, const List &args)
{
  Value result;
  String s;
  DOM::HTMLElement element;
  DOM::HTMLCollection coll;

  Value v = args[0];

  switch (id) {
  case HTMLDocument::Images:
    coll = doc.images();
    break;
  case HTMLDocument::Applets:
    coll = doc.applets();
    break;
  case HTMLDocument::Links:
    coll = doc.links();
    break;
  case HTMLDocument::Forms:
    coll = doc.forms();
    break;
  case HTMLDocument::Anchors:
    coll = doc.anchors();
    break;
  case HTMLDocument::All:
    coll = doc.all();
    break;
  case HTMLDocument::Open:
    // this is just a dummy function,  has no purpose anymore
    //doc.open();
    result = Undefined();
    break;
  case HTMLDocument::Close:
    // this is just a dummy function,  has no purpose
    // see khtmltests/ecma/tokenizer-script-recursion.html
    // doc.close();
    result = Undefined();
    break;
  case HTMLDocument::Write:
  case HTMLDocument::WriteLn: {
    // DOM only specifies single string argument, but NS & IE allow multiple
    UString str = v.toString(exec);
    for (int i = 1; i < args.size(); i++)
      str += args[i].toString(exec);
    if (id == HTMLDocument::WriteLn)
      str += "\n";
    doc.write(str.string());
    result = Undefined();
    break;
  }
  case HTMLDocument::GetElementById:
    result = getDOMNode(exec,doc.getElementById(v.toString(exec).string()));
    break;
  case HTMLDocument::GetElementsByName:
    result = getDOMNodeList(exec,doc.getElementsByName(v.toString(exec).string()));
    break;
  }

  // retrieve element from collection. Either by name or indexed.
  if (id == HTMLDocument::Images || id == HTMLDocument::Applets || id == HTMLDocument::Links ||
      id == HTMLDocument::Forms || id == HTMLDocument::Anchors || id == HTMLDocument::All) {
    bool ok;
    UString s = args[0].toString(exec);
    unsigned int u = s.toULong(&ok);
    if (ok)
      element = coll.item(u);
    else
      element = coll.namedItem(s.string());
    result = getDOMNode(exec,element);
  }

  return result;
}

const ClassInfo KJS::HTMLDocument::info =
  { "HTMLDocument", &DOMDocument::info, &HTMLDocumentTable, 0 };
/* Source for HTMLDocumentTable. Use "make hashtables" to regenerate.
@begin HTMLDocumentTable 31
  title			HTMLDocument::Title		DontDelete
  referrer		HTMLDocument::Referrer		DontDelete|ReadOnly
  domain		HTMLDocument::Domain		DontDelete|ReadOnly
  URL			HTMLDocument::URL		DontDelete|ReadOnly
  body			HTMLDocument::Body		DontDelete
  location		HTMLDocument::Location		DontDelete
  cookie		HTMLDocument::Cookie		DontDelete
  images		HTMLDocument::Images		DontDelete|Function 0
  applets		HTMLDocument::Applets		DontDelete|Function 0
  links			HTMLDocument::Links		DontDelete|Function 0
  forms			HTMLDocument::Forms		DontDelete|Function 0
  anchors		HTMLDocument::Anchors		DontDelete|Function 0
  all			HTMLDocument::All		DontDelete|Function 0
  open			HTMLDocument::Open		DontDelete|Function 0
  close			HTMLDocument::Close		DontDelete|Function 0
  write			HTMLDocument::Write		DontDelete|Function 1
  writeln		HTMLDocument::WriteLn		DontDelete|Function 1
  getElementById	HTMLDocument::GetElementById	DontDelete|Function 1
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
bool KJS::HTMLDocument::hasProperty(ExecState *exec, const UString &p, bool recursive) const
{
#ifdef KJS_VERBOSE
  kdDebug() << "KJS::HTMLDocument::hasProperty " << p.qstring() << endl;
#endif
  if (!static_cast<DOM::HTMLDocument>(node).all().
      namedItem(p.string()).isNull())
    return true;
  return DOMDocument::hasProperty(exec, p, recursive);
}

Value KJS::HTMLDocument::tryGet(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug() << "KJS::HTMLDocument::tryGet " << propertyName.qstring() << endl;
#endif
  DOM::HTMLDocument doc = static_cast<DOM::HTMLDocument>(node);
  DOM::HTMLBodyElement body = doc.body();

  KHTMLPart *part = static_cast<DOM::DocumentImpl*>(doc.handle())->
		      view()->part();

  // image and form elements with the name p will be looked up first
  DOM::HTMLCollection coll = doc.all();
  DOM::HTMLElement element = coll.namedItem(propertyName.string());
  if (!element.isNull() &&
      (element.elementId() == ID_IMG || element.elementId() == ID_FORM))
    return getDOMNode(exec,element);

  const HashEntry* entry = Lookup::findEntry(&HTMLDocumentTable, propertyName);
  if (entry) {
    switch (entry->value) {
    case Title:
      return getString(doc.title());
    case Referrer:
      return String(doc.referrer());
    case Domain:
    return String(doc.domain());
    case URL:
      return getString(doc.URL());
    case Body:
      return getDOMNode(exec,doc.body());
    case Location:
      return Window::retrieveWindow(part)->location();
    case Cookie:
      return String(doc.cookie());
    case Images:
    case Applets:
    case Links:
    case Forms:
    case Anchors:
    case All:
    case Open:
    case Close:
    case Write:
    case WriteLn:
    case GetElementById:
    case GetElementsByName:
      //return lookupOrCreateFunction<HTMLDocFunction,HTMLDocument>( exec, propertyName, this, entry );
      // Modified copy of lookupOrCreateFunction because the ctor needs 'doc'
      ValueImp * cachedVal = ObjectImp::getDirect(propertyName);
      if (cachedVal && cachedVal->type() == ObjectType)
        return cachedVal;
      Value val = new HTMLDocFunction(exec, doc, entry->value, entry->params);
      const_cast<HTMLDocument *>(this)->ObjectImp::put(exec, propertyName, val, entry->attr);
      return val;
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
      return Number(part->view() ? part->view()->visibleWidth() : 0);
    case Width:
      return Number(part->view() ? part->view()->visibleWidth() : 0);
    case Dir:
      return String(body.dir());
    }
  }
  if (DOMDocument::hasProperty(exec, propertyName, true))
    return DOMDocument::tryGet(exec, propertyName);

  //kdDebug() << "KJS::HTMLDocument::tryGet " << propertyName.qstring() << " not found, returning element" << endl;
  if(!element.isNull())
    return getDOMNode(exec,element);
  return Undefined();
}

void KJS::HTMLDocument::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
#ifdef KJS_VERBOSE
  kdDebug() << "KJS::HTMLDocument::tryPut " << propertyName.qstring() << endl;
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
  case Cookie:
    doc.setCookie(value.toString(exec).string());
    break;
  case Location: {
    KHTMLPart *part = static_cast<DOM::DocumentImpl *>( doc.handle() )->view()->part();
    QString str = value.toString(exec).qstring();
    part->scheduleRedirection(0, str);
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

const ClassInfo KJS::HTMLElement::info = { "HTMLElement",
					  &DOMElement::info, 0, 0 };

Value KJS::HTMLElement::tryGet(ExecState *exec, const UString &p) const
{
  DOM::HTMLElement element = static_cast<DOM::HTMLElement>(node);
#ifdef KJS_VERBOSE
  kdDebug() << "KJS::HTMLElement::tryGet " << p.qstring() << " id=" << element.elementId() << endl;
#endif

  switch (element.elementId()) {
    case ID_HTML: {
      DOM::HTMLHtmlElement html = element;
      if      (p == "version")         return getString(html.version());
    }
    break;
    case ID_HEAD: {
      DOM::HTMLHeadElement head = element;
      if      (p == "profile")         return getString(head.profile());
    }
    break;
    case ID_LINK: {
      DOM::HTMLLinkElement link = element;
      if      (p == "disabled")        return Boolean(link.disabled());
      else if (p == "charset")         return getString(link.charset());
      else if (p == "href")            return getString(link.href());
      else if (p == "hreflang")        return getString(link.hreflang());
      else if (p == "media")           return getString(link.media());
      else if (p == "rel")             return getString(link.rel());
      else if (p == "rev")             return getString(link.rev());
      else if (p == "target")          return getString(link.target());
      else if (p == "type")            return getString(link.type());
      else if (p == "sheet")           return getDOMStyleSheet(exec,static_cast<DOM::ProcessingInstruction>(node).sheet());
    }
    break;
    case ID_TITLE: {
      DOM::HTMLTitleElement title = element;
      if (p == "text")                 return getString(title.text());
    }
    break;
    case ID_META: {
      DOM::HTMLMetaElement meta = element;
      if      (p == "content")         return getString(meta.content());
      else if (p == "httpEquiv")       return getString(meta.httpEquiv());
      else if (p == "name")            return getString(meta.name());
      else if (p == "scheme")          return getString(meta.scheme());
    }
    break;
    case ID_BASE: {
      DOM::HTMLBaseElement base = element;
      if      (p == "href")            return getString(base.href());
      else if (p == "target")          return getString(base.target());
    }
    break;
    case ID_ISINDEX: {
      DOM::HTMLIsIndexElement isindex = element;
      if      (p == "form")            return getDOMNode(exec,isindex.form()); // type HTMLFormElement
      else if (p == "prompt")          return getString(isindex.prompt());
    }
    break;
    case ID_STYLE: {
      DOM::HTMLStyleElement style = element;
      if      (p == "disabled")        return Boolean(style.disabled());
      else if (p == "media")           return getString(style.media());
      else if (p == "type")            return getString(style.type());
      else if (p == "sheet")           return getDOMStyleSheet(exec,static_cast<DOM::ProcessingInstruction>(node).sheet());
    }
    break;
    case ID_BODY: {
      DOM::HTMLBodyElement body = element;
      if      (p == "aLink")           return getString(body.aLink());
      else if (p == "background")      return getString(body.background());
      else if (p == "bgColor")         return getString(body.bgColor());
      else if (p == "link")            return getString(body.link());
      else if (p == "text")            return getString(body.text());
      else if (p == "vLink")           return getString(body.vLink());
      else if (p == "scrollHeight" )   return Number(body.ownerDocument().view() ? body.ownerDocument().view()->contentsHeight() : 0);
      else if (p == "scrollWidth" )    return Number(body.ownerDocument().view() ? body.ownerDocument().view()->contentsWidth() : 0);
    }
    break;
    case ID_FORM: {
      DOM::HTMLFormElement form = element;
      DOM::Node n = form.elements().namedItem(p.string());
      if(!n.isNull())  return getDOMNode(exec,n);
      else if (p == "elements")        return getHTMLCollection(exec,form.elements());
      else if (p == "length")          return Number(form.length());
      else if (p == "name")            return getString(form.name());
      else if (p == "acceptCharset")   return getString(form.acceptCharset());
      else if (p == "action")          return getString(form.action());
      else if (p == "enctype")         return getString(form.enctype());
      else if (p == "method")          return getString(form.method());
      else if (p == "target")          return getString(form.target());
      // methods
      else if (p == "submit")          return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Submit,0,DontDelete|Function);
      else if (p == "reset")           return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Reset,0,DontDelete|Function);
      else
        return DOMElement::tryGet(exec, p);
    }
    break;
    case ID_SELECT: {
      DOM::HTMLSelectElement select = element;
      if      (p == "type")            return getString(select.type());
      else if (p == "selectedIndex")   return Number(select.selectedIndex());
      else if (p == "value")           return getString(select.value());
      else if (p == "length")          return Number(select.length());
      else if (p == "form")            return getDOMNode(exec,select.form()); // type HTMLFormElement
      else if (p == "options")         return getSelectHTMLCollection(exec, select.options(), select); // type HTMLCollection
      else if (p == "disabled")        return Boolean(select.disabled());
      else if (p == "multiple")        return Boolean(select.multiple());
      else if (p == "name")            return getString(select.name());
      else if (p == "size")            return Number(select.size());
      else if (p == "tabIndex")        return Number(select.tabIndex());
      // methods
      else if (p == "add")             return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Add,2,DontDelete|Function);
      else if (p == "remove")          return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Remove,1,DontDelete|Function);
      else if (p == "blur")            return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Blur,0,DontDelete|Function);
      else if (p == "focus")           return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Focus,0,DontDelete|Function);
      else {
	bool ok;
	uint u = p.toULong(&ok);
	if (ok)
	  return getDOMNode(exec,select.options().item(u)); // not specified by DOM(?) but supported in netscape/IE
      }
    }
    break;
    case ID_OPTGROUP: {
      DOM::HTMLOptGroupElement optgroup = element;
      if      (p == "disabled")        return Boolean(optgroup.disabled());
      else if (p == "label")           return getString(optgroup.label());
    }
    break;
    case ID_OPTION: {
      DOM::HTMLOptionElement option = element;
      if      (p == "form")            return getDOMNode(exec,option.form()); // type HTMLFormElement
      else if (p == "defaultSelected") return Boolean(option.defaultSelected());
      else if (p == "text")            return getString(option.text());
      else if (p == "index")           return Number(option.index());
      else if (p == "disabled")        return Boolean(option.disabled());
      else if (p == "label")           return getString(option.label());
      else if (p == "selected")        return Boolean(option.selected());
      else if (p == "value")           return getString(option.value());
    }
    break;
    case ID_INPUT: {
      DOM::HTMLInputElement input = element;
      if      (p == "defaultValue")    return getString(input.defaultValue());
      else if (p == "defaultChecked")  return Boolean(input.defaultChecked());
      else if (p == "form")            return getDOMNode(exec,input.form()); // type HTMLFormElement
      else if (p == "accept")          return getString(input.accept());
      else if (p == "accessKey")       return getString(input.accessKey());
      else if (p == "align")           return getString(input.align());
      else if (p == "alt")             return getString(input.alt());
      else if (p == "checked")         return Boolean(input.checked());
      else if (p == "disabled")        return Boolean(input.disabled());

      else if (p == "length") {
        // SLOOOOOOW
        DOM::HTMLCollection c( input.form().elements() );
        unsigned long len = 0;
        for ( unsigned long i = 0; i < c.length(); i++ )
          if ( static_cast<DOM::Element>( c.item( i ) ).getAttribute( "name" ) == input.name() )
            len++;
        return Number(len);
      }
      else if (p == "maxLength")       return Number(input.maxLength());
      else if (p == "name")            return getString(input.name());
      else if (p == "readOnly")        return Boolean(input.readOnly());
      else if (p == "size")            return getString(input.size());
      else if (p == "src")             return getString(input.src());
      else if (p == "tabIndex")        return Number(input.tabIndex());
      else if (p == "type")            return getString(input.type());
      else if (p == "useMap")          return getString(input.useMap());
      else if (p == "value")           return getString(input.value());
      // methods
      else if (p == "blur")            return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Blur,0,DontDelete|Function);
      else if (p == "focus")           return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Focus,0,DontDelete|Function);
      else if (p == "select")          return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Select,0,DontDelete|Function);
      else if (p == "click")           return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Click,0,DontDelete|Function);
      else {
        // ### SLOOOOOOOW
        bool ok;
        uint u = p.toULong(&ok);
        if ( !ok ) break;

        DOM::HTMLCollection c( input.form().elements() );
         for ( unsigned long i = 0; i < c.length(); i++ )
           if ( static_cast<DOM::Element>( c.item( i ) ).getAttribute( "name" ) == input.name() )
             if ( u-- == 0 )
               return getDOMNode(exec, c.item( i ) );
      }
    }
    break;
    case ID_TEXTAREA: {
      DOM::HTMLTextAreaElement textarea = element;
      if      (p == "defaultValue")    return getString(textarea.defaultValue());
      else if (p == "form")            return getDOMNode(exec,textarea.form()); // type HTMLFormElement
      else if (p == "accessKey")       return getString(textarea.accessKey());
      else if (p == "cols")            return Number(textarea.cols());
      else if (p == "disabled")        return Boolean(textarea.disabled());
      else if (p == "name")            return getString(textarea.name());
      else if (p == "readOnly")        return Boolean(textarea.readOnly());
      else if (p == "rows")            return Number(textarea.rows());
      else if (p == "tabIndex")        return Number(textarea.tabIndex());
      else if (p == "type")            return getString(textarea.type());
      else if (p == "value")           return getString(textarea.value());
      // methods
      else if (p == "blur")            return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Blur,0,DontDelete|Function);
      else if (p == "focus")           return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Focus,0,DontDelete|Function);
      else if (p == "select")          return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Select,0,DontDelete|Function);
    }
    break;
    case ID_BUTTON: {
      DOM::HTMLButtonElement button = element;
      if      (p == "form")            return getDOMNode(exec,button.form()); // type HTMLFormElement
      else if (p == "accessKey")       return getString(button.accessKey());
      else if (p == "disabled")        return Boolean(button.disabled());
      else if (p == "name")            return getString(button.name());
      else if (p == "tabIndex")        return Number(button.tabIndex());
      else if (p == "type")            return getString(button.type());
      else if (p == "value")           return getString(button.value());
    }
    break;
    case ID_LABEL: {
      DOM::HTMLLabelElement label = element;
      if      (p == "form")            return getDOMNode(exec,label.form()); // type HTMLFormElement
      else if (p == "accessKey")       return getString(label.accessKey());
      else if (p == "htmlFor")         return getString(label.htmlFor());
    }
    break;
    case ID_FIELDSET: {
      DOM::HTMLFieldSetElement fieldSet = element;
      if      (p == "form")            return getDOMNode(exec,fieldSet.form()); // type HTMLFormElement
    }
    break;
    case ID_LEGEND: {
      DOM::HTMLLegendElement legend = element;
      if      (p == "form")            return getDOMNode(exec,legend.form()); // type HTMLFormElement
      else if (p == "accessKey")       return getString(legend.accessKey());
      else if (p == "align")           return getString(legend.align());
    }
    break;
    case ID_UL: {
      DOM::HTMLUListElement uList = element;
      if      (p == "compact")         return Boolean(uList.compact());
      else if (p == "type")            return getString(uList.type());
    }
    break;
    case ID_OL: {
      DOM::HTMLOListElement oList = element;
      if      (p == "compact")         return Boolean(oList.compact());
      else if (p == "start")           return Number(oList.start());
      else if (p == "type")            return getString(oList.type());
    }
    break;
    case ID_DL: {
      DOM::HTMLDListElement dList = element;
      if      (p == "compact")         return Boolean(dList.compact());
    }
    break;
    case ID_DIR: {
      DOM::HTMLDirectoryElement directory = element;
      if      (p == "compact")         return Boolean(directory.compact());
    }
    break;
    case ID_MENU: {
      DOM::HTMLMenuElement menu = element;
      if      (p == "compact")         return Boolean(menu.compact());
    }
    break;
    case ID_LI: {
      DOM::HTMLLIElement li = element;
      if      (p == "type")            return getString(li.type());
      else if (p == "value")           return Number(li.value());
    }
    break;
    case ID_DIV: {
      DOM::HTMLDivElement div = element;
      if      (p == "align")           return getString(div.align());
    }
    break;
    case ID_P: {
      DOM::HTMLParagraphElement paragraph = element;
      if      (p == "align")           return getString(paragraph.align());
    }
    break;
    case ID_H1: { // ### H2, H3 ,H4 ,H5 ,H6
      DOM::HTMLHeadingElement heading = element;
      if      (p == "align")           return getString(heading.align());
    }
    break;
    case ID_BLOCKQUOTE: {
      DOM::HTMLBlockquoteElement blockquote = element;
      if      (p == "cite")            return getString(blockquote.cite());
    }
    case ID_Q: {
      DOM::HTMLQuoteElement quote = element;
      if      (p == "cite")            return getString(quote.cite());
    }
    case ID_PRE: {
      DOM::HTMLPreElement pre = element;
      if      (p == "width")           return Number(pre.width());
    }
    break;
    case ID_BR: {
      DOM::HTMLBRElement br = element;
      if      (p == "clear")           return getString(br.clear());
    }
    break;
    case ID_BASEFONT: {
      DOM::HTMLBaseFontElement baseFont = element;
      if      (p == "color")           return getString(baseFont.color());
      else if (p == "face")            return getString(baseFont.face());
      else if (p == "size")            return getString(baseFont.size());
    }
    break;
    case ID_FONT: {
      DOM::HTMLFontElement font = element;
      if      (p == "color")           return getString(font.color());
      else if (p == "face")            return getString(font.face());
      else if (p == "size")            return getString(font.size());
    }
    break;
    case ID_HR: {
      DOM::HTMLHRElement hr = element;
      if      (p == "align")           return getString(hr.align());
      else if (p == "noShade")         return Boolean(hr.noShade());
      else if (p == "size")            return getString(hr.size());
      else if (p == "width")           return getString(hr.width());
    }
    break;
    case ID_INS:
    case ID_DEL: {
      DOM::HTMLModElement mod = element;
      if      (p == "cite")            return getString(mod.cite());
      else if (p == "dateTime")        return getString(mod.dateTime());
    }
    break;
    case ID_A: {
      DOM::HTMLAnchorElement anchor = element;
      if      (p == "accessKey")       return getString(anchor.accessKey());
      else if (p == "charset")         return getString(anchor.charset());
      else if (p == "coords")          return getString(anchor.coords());
      else if (p == "href")            return getString(anchor.href());
      else if (p == "hreflang")        return getString(anchor.hreflang());
      else if (p == "hash")            return getString('#'+KURL(anchor.href().string()).ref());
      else if (p == "host")            return getString(KURL(anchor.href().string()).host());
      else if (p == "hostname") {
        KURL url(anchor.href().string());
        kdDebug(6070) << "anchor::hostname uses:" <<url.url()<<endl;
        if (url.port()==0)
          return getString(url.host());
        else
          return getString(url.host() + ":" + QString::number(url.port()));
      }
      else if (p == "pathname")        return getString(KURL(anchor.href().string()).path());
      else if (p == "port")            return getString(QString::number(KURL(anchor.href().string()).port()));
      else if (p == "protocol")        return getString(KURL(anchor.href().string()).protocol()+":");
      else if (p == "search")          return getString(KURL(anchor.href().string()).query());
      else if (p == "name")            return getString(anchor.name());
      else if (p == "rel")             return getString(anchor.rel());
      else if (p == "rev")             return getString(anchor.rev());
      else if (p == "shape")           return getString(anchor.shape());
      else if (p == "tabIndex")        return Number(anchor.tabIndex());
      else if (p == "target")          return getString(anchor.target());
      else if (p == "text")            return getString(anchor.innerHTML());
      else if (p == "type")            return getString(anchor.type());
      // methods
      else if (p == "blur")            return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Blur,0,DontDelete|Function);
      else if (p == "focus")           return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::Focus,0,DontDelete|Function);
    }
    break;
    case ID_IMG: {
      DOM::HTMLImageElement image = element;
      if      (p == "lowSrc")          return getString(image.lowSrc());
      else if (p == "name")            return getString(image.name());
      else if (p == "align")           return getString(image.align());
      else if (p == "alt")             return getString(image.alt());
      else if (p == "border")          return getString(image.border());
      else if (p == "height")          return getString(image.height());
      else if (p == "hspace")          return getString(image.hspace());
      else if (p == "isMap")           return Boolean(image.isMap());
      else if (p == "longDesc")        return getString(image.longDesc());
      else if (p == "src")             return getString(image.src());
      else if (p == "useMap")          return getString(image.useMap());
      else if (p == "vspace")          return getString(image.vspace());
      else if (p == "width")           return getString(image.width());
    }
    break;
    case ID_OBJECT: {
      DOM::HTMLObjectElement object = element;
      if      (p == "form")            return getDOMNode(exec,object.form()); // type HTMLFormElement
      else if (p == "code")            return getString(object.code());
      else if (p == "align")           return getString(object.align());
      else if (p == "archive")         return getString(object.archive());
      else if (p == "border")          return getString(object.border());
      else if (p == "codeBase")        return getString(object.codeBase());
      else if (p == "codeType")        return getString(object.codeType());
      else if (p == "data")            return getString(object.data());
      else if (p == "declare")         return Boolean(object.declare());
      else if (p == "height")          return getString(object.height());
      else if (p == "hspace")          return getString(object.hspace());
      else if (p == "name")            return getString(object.name());
      else if (p == "standby")         return getString(object.standby());
      else if (p == "tabIndex")        return Number(object.tabIndex());
      else if (p == "type")            return getString(object.type());
      else if (p == "useMap")          return getString(object.useMap());
      else if (p == "vspace")          return getString(object.vspace());
      else if (p == "width")           return getString(object.width());
//      else if (p == "contentDocument") // new for DOM2 - not yet in khtml
//        return getDOMNode(exec,object.contentDocument()); // type Document
    }
    break;
    case ID_PARAM: {
      DOM::HTMLParamElement param = element;
      if      (p == "name")            return getString(param.name());
      else if (p == "type")            return getString(param.type());
      else if (p == "value")           return getString(param.value());
      else if (p == "valueType")       return getString(param.valueType());
    }
    break;
    case ID_APPLET: {
      DOM::HTMLAppletElement applet = element;
      if      (p == "align")           return getString(applet.align());
      else if (p == "alt")             return getString(applet.alt());
      else if (p == "archive")         return getString(applet.archive());
      else if (p == "code")            return getString(applet.code());
      else if (p == "codeBase")        return getString(applet.codeBase());
      else if (p == "height")          return getString(applet.height());
      else if (p == "hspace")          return getString(applet.hspace());
      else if (p == "name")            return getString(applet.name());
      else if (p == "object")          return getString(applet.object());
      else if (p == "vspace")          return getString(applet.vspace());
      else if (p == "width")           return getString(applet.width());
    }
    break;
    case ID_MAP: {
      DOM::HTMLMapElement map = element;
      if      (p == "areas")           return getHTMLCollection(exec, map.areas()); // type HTMLCollection
      else if (p == "name")            return getString(map.name());
    }
    break;
    case ID_AREA: {
      DOM::HTMLAreaElement area = element;
      if      (p == "accessKey")       return getString(area.accessKey());
      else if (p == "alt")             return getString(area.alt());
      else if (p == "coords")          return getString(area.coords());
      else if (p == "href")            return getString(area.href());
      else if (p == "hash")            return getString('#'+KURL(area.href().string()).ref());
      else if (p == "host")            return getString(KURL(area.href().string()).host());
      else if (p == "hostname") {
        KURL url(area.href().string());
        kdDebug(6070) << "link::hostname uses:" <<url.url()<<endl;
        if (url.port()==0)
          return getString(url.host());
        else
          return getString(url.host() + ":" + QString::number(url.port()));
      }
      else if (p == "pathname")        return getString(KURL(area.href().string()).path());
      else if (p == "port")            return getString(QString::number(KURL(area.href().string()).port()));
      else if (p == "protocol")        return getString(KURL(area.href().string()).protocol()+":");
      else if (p == "search")          return getString(KURL(area.href().string()).query());

      else if (p == "noHref")          return Boolean(area.noHref());
      else if (p == "shape")           return getString(area.shape());
      else if (p == "tabIndex")        return Number(area.tabIndex());
      else if (p == "target")          return getString(area.target());
    }
    break;
    case ID_SCRIPT: {
      DOM::HTMLScriptElement script = element;
      if      (p == "text")            return getString(script.text());
      else if (p == "htmlFor")         return getString(script.htmlFor());
      else if (p == "event")           return getString(script.event());
      else if (p == "charset")         return getString(script.charset());
      else if (p == "defer")           return Boolean(script.defer());
      else if (p == "src")             return getString(script.src());
      else if (p == "type")            return getString(script.type());
    }
    break;
    case ID_TABLE: {
      DOM::HTMLTableElement table = element;
      if      (p == "caption")         return getDOMNode(exec,table.caption()); // type HTMLTableCaptionElement
      else if (p == "tHead")           return getDOMNode(exec,table.tHead()); // type HTMLTableSectionElement
      else if (p == "tFoot")           return getDOMNode(exec,table.tFoot()); // type HTMLTableSectionElement
      else if (p == "rows")            return getHTMLCollection(exec,table.rows()); // type HTMLCollection
      else if (p == "tBodies")         return getHTMLCollection(exec,table.tBodies()); // type HTMLCollection
      else if (p == "align")           return getString(table.align());
      else if (p == "bgColor")         return getString(table.bgColor());
      else if (p == "border")          return getString(table.border());
      else if (p == "cellPadding")     return getString(table.cellPadding());
      else if (p == "cellSpacing")     return getString(table.cellSpacing());
      else if (p == "frame")           return getString(table.frame());
      else if (p == "rules")           return getString(table.rules());
      else if (p == "summary")         return getString(table.summary());
      else if (p == "width")           return getString(table.width());
      // methods
      else if (p == "createTHead")     return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::CreateTHead,0,DontDelete|Function);
      else if (p == "deleteTHead")     return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::DeleteTHead,0,DontDelete|Function);
      else if (p == "createTFoot")     return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::CreateTFoot,0,DontDelete|Function);
      else if (p == "deleteTFoot")     return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::DeleteTFoot,0,DontDelete|Function);
      else if (p == "createCaption")   return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::CreateCaption,0,DontDelete|Function);
      else if (p == "deleteCaption")   return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::DeleteCaption,0,DontDelete|Function);
      else if (p == "insertRow")       return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::InsertRow,1,DontDelete|Function);
      else if (p == "deleteRow")       return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::DeleteRow,1,DontDelete|Function);
    }
    break;
    case ID_CAPTION: {
      DOM::HTMLTableCaptionElement tableCaption;
      if      (p == "align")           return getString(tableCaption.align());
    }
    break;
    case ID_COL: {
      DOM::HTMLTableColElement tableCol = element;
      if      (p == "align")           return getString(tableCol.align());
      else if (p == "ch")              return getString(tableCol.ch());
      else if (p == "chOff")           return getString(tableCol.chOff());
      else if (p == "span")            return Number(tableCol.span());
      else if (p == "vAlign")          return getString(tableCol.vAlign());
      else if (p == "width")           return getString(tableCol.width());
    }
    break;
    case ID_THEAD:
    case ID_TBODY:
    case ID_TFOOT: {
      DOM::HTMLTableSectionElement tableSection = element;
      if      (p == "align")           return getString(tableSection.align());
      else if (p == "ch")              return getString(tableSection.ch());
      else if (p == "chOff")           return getString(tableSection.chOff());
      else if (p == "vAlign")          return getString(tableSection.vAlign());
      else if (p == "rows")            return getHTMLCollection(exec,tableSection.rows()); // type HTMLCollection
      // methods
      else if (p == "insertRow")       return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::InsertRow,1,DontDelete|Function);
      else if (p == "deleteRow")       return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::DeleteRow,1,DontDelete|Function);
    }
    break;
    case ID_TR: {
      DOM::HTMLTableRowElement tableRow = element;
      if      (p == "rowIndex")        return Number(tableRow.rowIndex());
      else if (p == "sectionRowIndex") return Number(tableRow.sectionRowIndex());
      else if (p == "cells")           return getHTMLCollection(exec,tableRow.cells()); // type HTMLCollection
      else if (p == "align")           return getString(tableRow.align());
      else if (p == "bgColor")         return getString(tableRow.bgColor());
      else if (p == "ch")              return getString(tableRow.ch());
      else if (p == "chOff")           return getString(tableRow.chOff());
      else if (p == "vAlign")          return getString(tableRow.vAlign());
      // methods
      else if (p == "insertCell")      return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::InsertCell,1,DontDelete|Function);
      else if (p == "deleteCell")      return lookupOrCreateFunction<HTMLElementFunction>(exec,p,this,HTMLElementFunction::DeleteCell,1,DontDelete|Function);
    }
    break;
    case ID_TH:
    case ID_TD: {
      DOM::HTMLTableCellElement tableCell = element;
      if      (p == "cellIndex")       return Number(tableCell.cellIndex());
      else if (p == "abbr")            return getString(tableCell.abbr());
      else if (p == "align")           return getString(tableCell.align());
      else if (p == "axis")            return getString(tableCell.axis());
      else if (p == "bgColor")         return getString(tableCell.bgColor());
      else if (p == "ch")              return getString(tableCell.ch());
      else if (p == "chOff")           return getString(tableCell.chOff());
      else if (p == "colSpan")         return Number(tableCell.colSpan());
      else if (p == "headers")         return getString(tableCell.headers());
      else if (p == "height")          return getString(tableCell.height());
      else if (p == "noWrap")          return Boolean(tableCell.noWrap());
      else if (p == "rowSpan")         return Number(tableCell.rowSpan());
      else if (p == "scope")           return getString(tableCell.scope());
      else if (p == "vAlign")          return getString(tableCell.vAlign());
      else if (p == "width")           return getString(tableCell.width());
    }
    break;
    case ID_FRAMESET: {
      DOM::HTMLFrameSetElement frameSet = element;
      if      (p == "cols")            return getString(frameSet.cols());
      else if (p == "rows")            return getString(frameSet.rows());
    }
    break;
    case ID_FRAME: {
      DOM::HTMLFrameElement frameElement = element;

      // p == "document" ?
      if (p == "frameBorder")          return getString(frameElement.frameBorder());
      else if (p == "longDesc")        return getString(frameElement.longDesc());
      else if (p == "marginHeight")    return getString(frameElement.marginHeight());
      else if (p == "marginWidth")     return getString(frameElement.marginWidth());
      else if (p == "name")            return getString(frameElement.name());
      else if (p == "noResize")        return Boolean(frameElement.noResize());
      else if (p == "scrolling")       return getString(frameElement.scrolling());
      else if (p == "src")             return getString(frameElement.src());
//      else if (p == "contentDocument") // new for DOM2 - not yet in khtml
//        return getDOMNode(exec,frameElement.contentDocument()); // type Document
    }
    break;
    case ID_IFRAME: {
      DOM::HTMLIFrameElement iFrame = element;
      if (p == "align")                return getString(iFrame.align());
      // ### security check ?
      else if (p == "document") {
        if ( !iFrame.isNull() )
          return getDOMNode(exec, static_cast<DOM::HTMLIFrameElementImpl*>(iFrame.handle() )->frameDocument() );

        return Undefined();
      }
      else if (p == "frameBorder")     return getString(iFrame.frameBorder());
      else if (p == "height")          return getString(iFrame.height());
      else if (p == "longDesc")        return getString(iFrame.longDesc());
      else if (p == "marginHeight")    return getString(iFrame.marginHeight());
      else if (p == "marginWidth")     return getString(iFrame.marginWidth());
      else if (p == "name")            return getString(iFrame.name());
      else if (p == "scrolling")       return getString(iFrame.scrolling());
      else if (p == "src")             return getString(iFrame.src());
      else if (p == "width")           return getString(iFrame.width());
//      else if (p == "contentDocument") // new for DOM2 - not yet in khtml
//        return getDOMNode(exec,iFrame.contentDocument); // type Document
    }
    break;
  }

  // generic properties
  if (p == "id")
    return getString(element.id());
  else if (p == "title")
    return getString(element.title());
  else if (p == "lang")
    return getString(element.lang());
  else if (p == "dir")
    return getString(element.dir());
  else if (p == "className") // ### isn't this "class" in the HTML spec?
    return getString(element.className());
  else if ( p == "innerHTML")
      return getString(element.innerHTML());
  else if ( p == "innerText")
      return getString(element.innerText());
  else if ( p == "document")
      return getDOMNode(exec,element.ownerDocument());
  // ### what about style? or is this used instead for DOM2 stylesheets?
  else
    return DOMElement::tryGet(exec, p);
}

bool KJS::HTMLElement::hasProperty(ExecState *exec, const UString &propertyName, bool recursive) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "HTMLElement::hasProperty " << propertyName.qstring() << endl;
#endif
    Value tmp = tryGet(exec, propertyName);
    if (!tmp.isA(UndefinedType)) {
      return true;
    }
    return DOMElement::hasProperty(exec, propertyName, recursive);
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
  DOM::HTMLElement element = static_cast<KJS::HTMLElement *>(thisObj.imp())->toElement();
  Value result;

  switch (element.elementId()) {
    case ID_FORM: {
      DOM::HTMLFormElement form = element;
      if (id == Submit) {
        form.submit();
        result = Undefined();
      }
      else if (id == Reset) {
        form.reset();
        result = Undefined();
      }
    }
    break;
    case ID_SELECT: {
      DOM::HTMLSelectElement select = element;
      if (id == Add) {
        select.add(KJS::toNode(args[0]),KJS::toNode(args[1]));
        result = Undefined();
      }
      else if (id == Remove) {
        select.remove(int(args[0].toNumber(exec)));
        result = Undefined();
      }
      else if (id == Blur) {
        select.blur();
        result = Undefined();
      }
      else if (id == Focus) {
        select.focus();
        result = Undefined();
      }
    }
    break;
    case ID_INPUT: {
      DOM::HTMLInputElement input = element;
      if (id == Blur) {
        input.blur();
        result = Undefined();
      }
      else if (id == Focus) {
        input.focus();
        result = Undefined();
      }
      else if (id == Select) {
        input.select();
        result = Undefined();
      }
      else if (id == Click) {
        input.click();
        result = Undefined();
      }
    }
    break;
    case ID_TEXTAREA: {
      DOM::HTMLTextAreaElement textarea = element;
      if (id == Blur) {
        textarea.blur();
        result = Undefined();
      }
      else if (id == Focus) {
        textarea.focus();
        result = Undefined();
      }
      else if (id == Select) {
        textarea.select();
        result = Undefined();
      }
    }
    break;
    case ID_A: {
      DOM::HTMLAnchorElement anchor = element;
      if (id == Blur) {
        anchor.blur();
        result = Undefined();
      }
      else if (id == Focus) {
        anchor.focus();
        result = Undefined();
      }
    }
    break;
    case ID_TABLE: {
      DOM::HTMLTableElement table = element;
      if (id == CreateTHead)
        result = getDOMNode(exec,table.createTHead());
      else if (id == DeleteTHead) {
        table.deleteTHead();
        result = Undefined();
      }
      else if (id == CreateTFoot)
        result = getDOMNode(exec,table.createTFoot());
      else if (id == DeleteTFoot) {
        table.deleteTFoot();
        result = Undefined();
      }
      else if (id == CreateCaption)
        result = getDOMNode(exec,table.createCaption());
      else if (id == DeleteCaption) {
        table.deleteCaption();
        result = Undefined();
      }
      else if (id == InsertRow)
        result = getDOMNode(exec,table.insertRow(args[0].toInteger(exec)));
      else if (id == DeleteRow) {
        table.deleteRow(args[0].toInteger(exec));
        result = Undefined();
      }
    }
    break;
    case ID_THEAD:
    case ID_TBODY:
    case ID_TFOOT: {
      DOM::HTMLTableSectionElement tableSection = element;
      if (id == InsertRow)
        result = getDOMNode(exec,tableSection.insertRow(args[0].toInteger(exec)));
      else if (id == DeleteRow) {
        tableSection.deleteRow(args[0].toInteger(exec));
        result = Undefined();
      }
    }
    break;
    case ID_TR: {
      DOM::HTMLTableRowElement tableRow = element;
      if (id == InsertCell)
        result = getDOMNode(exec,tableRow.insertCell(args[0].toInteger(exec)));
      else if (id == DeleteCell) {
        tableRow.deleteCell(args[0].toInteger(exec));
        result = Undefined();
      }
    }
    break;
  }

  return result;
}

void KJS::HTMLElement::tryPut(ExecState *exec, const UString &p, const Value& v, int attr)
{
  DOM::DOMString str = v.isA(NullType) ? DOM::DOMString(0) : v.toString(exec).string();
  DOM::Node n = (new DOMNode(exec, KJS::toNode(v)))->toNode();
  DOM::HTMLElement element = static_cast<DOM::HTMLElement>(node);
#ifdef KJS_VERBOSE
  kdDebug() << "KJS::HTMLElement::tryPut " << p.qstring() << " id=" << element.elementId() << " str=" << str.string() << endl;
#endif

  switch (element.elementId()) {
    case ID_HTML: {
      DOM::HTMLHtmlElement html = element;
      if      (p == "version")         { html.setVersion(str); return; }
    }
    break;
    case ID_HEAD: {
      DOM::HTMLHeadElement head = element;
      if      (p == "profile")         { head.setProfile(str); return; }
    }
    break;
    case ID_LINK: {
      DOM::HTMLLinkElement link = element;
      if      (p == "disabled")        { link.setDisabled(v.toBoolean(exec)); return; }
      else if (p == "charset")         { link.setCharset(str); return; }
      else if (p == "href")            { link.setHref(str); return; }
      else if (p == "hreflang")        { link.setHreflang(str); return; }
      else if (p == "media")           { link.setMedia(str); return; }
      else if (p == "rel")             { link.setRel(str); return; }
      else if (p == "rev")             { link.setRev(str); return; }
      else if (p == "target")          { link.setTarget(str); return; }
      else if (p == "type")            { link.setType(str); return; }
    }
    break;
    case ID_TITLE: {
      DOM::HTMLTitleElement title = element;
      if (p == "text")                 { title.setText(str); return; }
    }
    break;
    case ID_META: {
      DOM::HTMLMetaElement meta = element;
      if      (p == "content")         { meta.setContent(str); return; }
      else if (p == "httpEquiv")       { meta.setHttpEquiv(str); return; }
      else if (p == "name")            { meta.setName(str); return; }
      else if (p == "scheme")          { meta.setScheme(str); return; }
    }
    break;
    case ID_BASE: {
      DOM::HTMLBaseElement base = element;
      if      (p == "href")            { base.setHref(str); return; }
      else if (p == "target")          { base.setTarget(str); return; }
    }
    break;
    case ID_ISINDEX: {
      DOM::HTMLIsIndexElement isindex = element;
      // read-only: form
      if (p == "prompt")               { isindex.setPrompt(str); return; }
    }
    break;
    case ID_STYLE: {
      DOM::HTMLStyleElement style = element;
      if      (p == "disabled")        { style.setDisabled(v.toBoolean(exec)); return; }
      else if (p == "media")           { style.setMedia(str); return; }
      else if (p == "type")            { style.setType(str); return; }
    }
    break;
    case ID_BODY: {
      DOM::HTMLBodyElement body = element;
      if      (p == "aLink")           { body.setALink(str); return; }
      else if (p == "background")      { body.setBackground(str); return; }
      else if (p == "bgColor")         { body.setBgColor(str); return; }
      else if (p == "link")            { body.setLink(str); return; }
      else if (p == "text")            { body.setText(str); return; }
      else if (p == "vLink")           { body.setVLink(str); return; }
    }
    break;
    case ID_FORM: {
      DOM::HTMLFormElement form = element;
      // read-only: elements
      // read-only: length
      if (p == "name")                 { form.setName(str); return; }
      else if (p == "acceptCharset")   { form.setAcceptCharset(str); return; }
      else if (p == "action") {
        form.setAction(str.string());
        return;
      }
      else if (p == "enctype")         { form.setEnctype(str); return; }
      else if (p == "method")          { form.setMethod(str); return; }
      else if (p == "target")          { form.setTarget(str); return; }
    }
    break;
    case ID_SELECT: {
      DOM::HTMLSelectElement select = element;
      // read-only: type
      if (p == "selectedIndex")        { select.setSelectedIndex(v.toInteger(exec)); return; }
      else if (p == "value")           { select.setValue(str); return; }
      else if (p == "length")          { // read-only according to the NS spec, but webpages need it writeable
                                         Object coll = Object::dynamicCast( getSelectHTMLCollection(exec, select.options(), select) );
                                         if ( !coll.isNull() )
                                           coll.put(exec,p,v);
                                         return;
                                       }
      // read-only: form
      // read-only: options
      else if (p == "disabled")        { select.setDisabled(v.toBoolean(exec)); return; }
      else if (p == "multiple")        { select.setMultiple(v.toBoolean(exec)); return; }
      else if (p == "name")            { select.setName(str); return; }
      else if (p == "size")            { select.setSize(v.toInteger(exec)); return; }
      else if (p == "tabIndex")        { select.setTabIndex(v.toInteger(exec)); return; }
    }
    break;
    case ID_OPTGROUP: {
      DOM::HTMLOptGroupElement optgroup = element;
      if      (p == "disabled")        { optgroup.setDisabled(v.toBoolean(exec)); return; }
      else if (p == "label")           { optgroup.setLabel(str); return; }
    }
    break;
    case ID_OPTION: {
      DOM::HTMLOptionElement option = element;
      // read-only: form
      if (p == "defaultSelected")      { option.setDefaultSelected(v.toBoolean(exec)); return; }
      // read-only: text  <--- According to the DOM, but JavaScript and JScript both allow changes.
      // So, we'll do it here and not add it to our DOM headers.
      else if (p == "text")            { DOM::NodeList nl(option.childNodes());
                                         for (unsigned int i = 0; i < nl.length(); i++) {
                                             if (nl.item(i).nodeType() == DOM::Node::TEXT_NODE) {
                                                 static_cast<DOM::Text>(nl.item(i)).setData(str);
                                                 return;
                                             }
                                         }
                                         kdWarning() << "HTMLElement::tryPut failed, no text node in option" << endl;
                                         return;
      }
      // read-only: index
      else if (p == "disabled")        { option.setDisabled(v.toBoolean(exec)); return; }
      else if (p == "label")           { option.setLabel(str); return; }
      else if (p == "selected")        { option.setSelected(v.toBoolean(exec)); return; }
      else if (p == "value")           { option.setValue(str); return; }
    }
    break;
    case ID_INPUT: {
      DOM::HTMLInputElement input = element;
      if      (p == "defaultValue")    { input.setDefaultValue(str); return; }
      else if (p == "defaultChecked")  { input.setDefaultChecked(v.toBoolean(exec)); return; }
      // read-only: form
      else if (p == "accept")          { input.setAccept(str); return; }
      else if (p == "accessKey")       { input.setAccessKey(str); return; }
      else if (p == "align")           { input.setAlign(str); return; }
      else if (p == "alt")             { input.setAlt(str); return; }
      else if (p == "checked")         { input.setChecked(v.toBoolean(exec)); return; }
      else if (p == "disabled")        { input.setDisabled(v.toBoolean(exec)); return; }
      else if (p == "maxLength")       { input.setMaxLength(v.toInteger(exec)); return; }
      else if (p == "name")            { input.setName(str); return; }
      else if (p == "readOnly")        { input.setReadOnly(v.toBoolean(exec)); return; }
      else if (p == "size")            { input.setSize(str); return; }
      else if (p == "src")             { input.setSrc(str); return; }
      else if (p == "tabIndex")        { input.setTabIndex(v.toInteger(exec)); return; }
      // read-only: type
      else if (p == "useMap")          { input.setUseMap(str); return; }
      else if (p == "value")           { input.setValue(str); return; }
    }
    break;
    case ID_TEXTAREA: {
      DOM::HTMLTextAreaElement textarea = element;
      if      (p == "defaultValue")    { textarea.setDefaultValue(str); return; }
      // read-only: form
      else if (p == "accessKey")       { textarea.setAccessKey(str); return; }
      else if (p == "cols")            { textarea.setCols(v.toInteger(exec)); return; }
      else if (p == "disabled")        { textarea.setDisabled(v.toBoolean(exec)); return; }
      else if (p == "name")            { textarea.setName(str); return; }
      else if (p == "readOnly")        { textarea.setReadOnly(v.toBoolean(exec)); return; }
      else if (p == "rows")            { textarea.setRows(v.toInteger(exec)); return; }
      else if (p == "tabIndex")        { textarea.setTabIndex(v.toInteger(exec)); return; }
      // read-only: type
      else if (p == "value")           { textarea.setValue(str); return; }
    }
    break;
    case ID_BUTTON: {
      DOM::HTMLButtonElement button = element;
      // read-only: form
      if (p == "accessKey")            { button.setAccessKey(str); return; }
      else if (p == "disabled")        { button.setDisabled(v.toBoolean(exec)); return; }
      else if (p == "name")            { button.setName(str); return; }
      else if (p == "tabIndex")        { button.setTabIndex(v.toInteger(exec)); return; }
      // read-only: type
      else if (p == "value")           { button.setValue(str); return; }
    }
    break;
    case ID_LABEL: {
      DOM::HTMLLabelElement label = element;
      // read-only: form
      if (p == "accessKey")            { label.setAccessKey(str); return; }
      else if (p == "htmlFor")         { label.setHtmlFor(str); return; }
    }
    break;
//    case ID_FIELDSET: {
//      DOM::HTMLFieldSetElement fieldSet = element;
//      // read-only: form
//    }
//    break;
    case ID_LEGEND: {
      DOM::HTMLLegendElement legend = element;
      // read-only: form
      if (p == "accessKey")            { legend.setAccessKey(str); return; }
      else if (p == "align")           { legend.setAlign(str); return; }
    }
    break;
    case ID_UL: {
      DOM::HTMLUListElement uList = element;
      if      (p == "compact")         { uList.setCompact(v.toBoolean(exec)); return; }
      else if (p == "type")            { uList.setType(str); return; }
    }
    break;
    case ID_OL: {
      DOM::HTMLOListElement oList = element;
      if      (p == "compact")         { oList.setCompact(v.toBoolean(exec)); return; }
      else if (p == "start")           { oList.setStart(v.toInteger(exec)); return; }
      else if (p == "type")            { oList.setType(str); return; }
    }
    break;
    case ID_DL: {
      DOM::HTMLDListElement dList = element;
      if      (p == "compact")         { dList.setCompact(v.toBoolean(exec)); return; }
    }
    break;
    case ID_DIR: {
      DOM::HTMLDirectoryElement directory = element;
      if      (p == "compact")         { directory.setCompact(v.toBoolean(exec)); return; }
    }
    break;
    case ID_MENU: {
      DOM::HTMLMenuElement menu = element;
      if      (p == "compact")         { menu.setCompact(v.toBoolean(exec)); return; }
    }
    break;
    case ID_LI: {
      DOM::HTMLLIElement li = element;
      if      (p == "type")            { li.setType(str); return; }
      else if (p == "value")           { li.setValue(v.toInteger(exec)); return; }
    }
    break;
    case ID_DIV: {
      DOM::HTMLDivElement div = element;
      if      (p == "align")           { div.setAlign(str); return; }
    }
    break;
    case ID_P: {
      DOM::HTMLParagraphElement paragraph = element;
      if      (p == "align")           { paragraph.setAlign(str); return; }
    }
    break;
    case ID_H1:
    case ID_H2:
    case ID_H3:
    case ID_H4:
    case ID_H5:
    case ID_H6: {
      DOM::HTMLHeadingElement heading = element;
      if      (p == "align")           { heading.setAlign(str); return; }
    }
    break;
    case ID_BLOCKQUOTE: {
      DOM::HTMLBlockquoteElement blockquote = element;
      if      (p == "cite")            { blockquote.setCite(str); return; }
    }
    case ID_Q: {
      DOM::HTMLQuoteElement quote = element;
      if      (p == "cite")            { quote.setCite(str); return; }
    }
    break;
    case ID_PRE: {
      DOM::HTMLPreElement pre = element;
      if      (p == "width")           { pre.setWidth(v.toInteger(exec)); return; }
    }
    break;
    case ID_BR: {
      DOM::HTMLBRElement br = element;
      if      (p == "clear")           { br.setClear(str); return; }
    }
    break;
    case ID_BASEFONT: {
      DOM::HTMLBaseFontElement baseFont = element;
      if      (p == "color")           { baseFont.setColor(str); return; }
      else if (p == "face")            { baseFont.setFace(str); return; }
      else if (p == "size")            { baseFont.setSize(str); return; }
    }
    break;
    case ID_FONT: {
      DOM::HTMLFontElement font = element;
      if      (p == "color")           { font.setColor(str); return; }
      else if (p == "face")            { font.setFace(str); return; }
      else if (p == "size")            { font.setSize(str); return; }
    }
    break;
    case ID_HR: {
      DOM::HTMLHRElement hr = element;
      if      (p == "align")           { hr.setAlign(str); return; }
      else if (p == "noShade")         { hr.setNoShade(v.toBoolean(exec)); return; }
      else if (p == "size")            { hr.setSize(str); return; }
      else if (p == "width")           { hr.setWidth(str); return; }
    }
    break;
    case ID_INS:
    case ID_DEL: {
      DOM::HTMLModElement mod = element;
      if      (p == "cite")            { mod.setCite(str); return; }
      else if (p == "dateTime")        { mod.setDateTime(str); return; }
    }
    break;
    case ID_A: {
      DOM::HTMLAnchorElement anchor = element;
      if      (p == "accessKey")       { anchor.setAccessKey(str); return; }
      else if (p == "charset")         { anchor.setCharset(str); return; }
      else if (p == "coords")          { anchor.setCoords(str); return; }
      else if (p == "href")            { anchor.setHref(str); return; }
      else if (p == "hreflang")        { anchor.setHreflang(str); return; }
      else if (p == "name")            { anchor.setName(str); return; }
      else if (p == "rel")             { anchor.setRel(str); return; }
      else if (p == "rev")             { anchor.setRev(str); return; }
      else if (p == "shape")           { anchor.setShape(str); return; }
      else if (p == "tabIndex")        { anchor.setTabIndex(v.toInteger(exec)); return; }
      else if (p == "target")          { anchor.setTarget(str); return; }
      else if (p == "type")            { anchor.setType(str); return; }
    }
    break;
    case ID_IMG: {
      DOM::HTMLImageElement image = element;
      if      (p == "lowSrc")          { image.setLowSrc(str); return; }
      else if (p == "name")            { image.setName(str); return; }
      else if (p == "align")           { image.setAlign(str); return; }
      else if (p == "alt")             { image.setAlt(str); return; }
      else if (p == "border")          { image.setBorder(str); return; }
      else if (p == "height")          { image.setHeight(str); return; }
      else if (p == "hspace")          { image.setHspace(str); return; }
      else if (p == "isMap")           { image.setIsMap(v.toBoolean(exec)); return; }
      else if (p == "longDesc")        { image.setLongDesc(str); return; }
      else if (p == "src")             { image.setSrc(str); return; }
      else if (p == "useMap")          { image.setUseMap(str); return; }
      else if (p == "vspace")          { image.setVspace(str); return; }
      else if (p == "width")           { image.setWidth(str); return; }
    }
    break;
    case ID_OBJECT: {
      DOM::HTMLObjectElement object = element;
      // read-only: form
      if (p == "code")                 { object.setCode(str); return; }
      else if (p == "align")           { object.setAlign(str); return; }
      else if (p == "archive")         { object.setArchive(str); return; }
      else if (p == "border")          { object.setBorder(str); return; }
      else if (p == "codeBase")        { object.setCodeBase(str); return; }
      else if (p == "codeType")        { object.setCodeType(str); return; }
      else if (p == "data")            { object.setData(str); return; }
      else if (p == "declare")         { object.setDeclare(v.toBoolean(exec)); return; }
      else if (p == "height")          { object.setHeight(str); return; }
      else if (p == "hspace")          { object.setHspace(str); return; }
      else if (p == "name")            { object.setName(str); return; }
      else if (p == "standby")         { object.setStandby(str); return; }
      else if (p == "tabIndex")        { object.setTabIndex(v.toInteger(exec)); return; }
      else if (p == "type")            { object.setType(str); return; }
      else if (p == "useMap")          { object.setUseMap(str); return; }
      else if (p == "vspace")          { object.setVspace(str); return; }
      else if (p == "width")           { object.setWidth(str); return; }
//      else if (p == "contentDocument") // new for DOM2 - not yet in khtml
//        return getDOMNode(exec,object.contentDocument()); // type Document
    }
    break;
    case ID_PARAM: {
      DOM::HTMLParamElement param = element;
      if      (p == "name")            { param.setName(str); return; }
      else if (p == "type")            { param.setType(str); return; }
      else if (p == "value")           { param.setValue(str); return; }
      else if (p == "valueType")       { param.setValueType(str); return; }
    }
    break;
    case ID_APPLET: {
      DOM::HTMLAppletElement applet = element;
      if      (p == "align")           { applet.setAlign(str); return; }
      else if (p == "alt")             { applet.setAlt(str); return; }
      else if (p == "archive")         { applet.setArchive(str); return; }
      else if (p == "code")            { applet.setCode(str); return; }
      else if (p == "codeBase")        { applet.setCodeBase(str); return; }
      else if (p == "height")          { applet.setHeight(str); return; }
      else if (p == "hspace")          { applet.setHspace(str); return; }
      else if (p == "name")            { applet.setName(str); return; }
      else if (p == "object")          { applet.setObject(str); return; }
      else if (p == "vspace")          { applet.setVspace(str); return; }
      else if (p == "width")           { applet.setWidth(str); return; }
    }
    break;
    case ID_MAP: {
      DOM::HTMLMapElement map = element;
      // read-only: areas
      if (p == "name")                 { map.setName(str); return; }
    }
    break;
    case ID_AREA: {
      DOM::HTMLAreaElement area = element;
      if      (p == "accessKey")       { area.setAccessKey(str); return; }
      else if (p == "alt")             { area.setAlt(str); return; }
      else if (p == "coords")          { area.setCoords(str); return; }
      else if (p == "href")            { area.setHref(str); return; }
      else if (p == "noHref")          { area.setNoHref(v.toBoolean(exec)); return; }
      else if (p == "shape")           { area.setShape(str); return; }
      else if (p == "tabIndex")        { area.setTabIndex(v.toInteger(exec)); return; }
      else if (p == "target")          { area.setTarget(str); return; }
    }
    break;
    case ID_SCRIPT: {
      DOM::HTMLScriptElement script = element;
      if      (p == "text")            { script.setText(str); return; }
      else if (p == "htmlFor")         { script.setHtmlFor(str); return; }
      else if (p == "event")           { script.setEvent(str); return; }
      else if (p == "charset")         { script.setCharset(str); return; }
      else if (p == "defer")           { script.setDefer(v.toBoolean(exec)); return; }
      else if (p == "src")             { script.setSrc(str); return; }
      else if (p == "type")            { script.setType(str); return; }
    }
    break;
    case ID_TABLE: {
      DOM::HTMLTableElement table = element;
      if      (p == "caption")         { table.setCaption(n); return; } // type HTMLTableCaptionElement
      else if (p == "tHead")           { table.setTHead(n); return; } // type HTMLTableSectionElement
      else if (p == "tFoot")           { table.setTFoot(n); return; } // type HTMLTableSectionElement
      // read-only: rows
      // read-only: tbodies
      else if (p == "align")           { table.setAlign(str); return; }
      else if (p == "bgColor")         { table.setBgColor(str); return; }
      else if (p == "border")          { table.setBorder(str); return; }
      else if (p == "cellPadding")     { table.setCellPadding(str); return; }
      else if (p == "cellSpacing")     { table.setCellSpacing(str); return; }
      else if (p == "frame")           { table.setFrame(str); return; }
      else if (p == "rules")           { table.setRules(str); return; }
      else if (p == "summary")         { table.setSummary(str); return; }
      else if (p == "width")           { table.setWidth(str); return; }
    }
    break;
    case ID_CAPTION: {
      DOM::HTMLTableCaptionElement tableCaption;
      if      (p == "align")           { tableCaption.setAlign(str); return; }
    }
    break;
    case ID_COL: {
      DOM::HTMLTableColElement tableCol = element;
      if      (p == "align")           { tableCol.setAlign(str); return; }
      else if (p == "ch")              { tableCol.setCh(str); return; }
      else if (p == "chOff")           { tableCol.setChOff(str); return; }
      else if (p == "span")            { tableCol.setSpan(v.toInteger(exec)); return; }
      else if (p == "vAlign")          { tableCol.setVAlign(str); return; }
      else if (p == "width")           { tableCol.setWidth(str); return; }
    }
    break;
    case ID_THEAD:
    case ID_TBODY:
    case ID_TFOOT: {
      DOM::HTMLTableSectionElement tableSection = element;
      if      (p == "align")           { tableSection.setAlign(str); return; }
      else if (p == "ch")              { tableSection.setCh(str); return; }
      else if (p == "chOff")           { tableSection.setChOff(str); return; }
      else if (p == "vAlign")          { tableSection.setVAlign(str); return; }
      // read-only: rows
    }
    break;
    case ID_TR: {
      DOM::HTMLTableRowElement tableRow = element;
      // read-only: rowIndex
      // read-only: sectionRowIndex
      // read-only: cells
      if      (p == "align")           { tableRow.setAlign(str); return; }
      else if (p == "bgColor")         { tableRow.setBgColor(str); return; }
      else if (p == "ch")              { tableRow.setCh(str); return; }
      else if (p == "chOff")           { tableRow.setChOff(str); return; }
      else if (p == "vAlign")          { tableRow.setVAlign(str); return; }
    }
    break;
    case ID_TH:
    case ID_TD: {
      DOM::HTMLTableCellElement tableCell = element;
      // read-only: cellIndex
      if      (p == "abbr")            { tableCell.setAbbr(str); return; }
      else if (p == "align")           { tableCell.setAlign(str); return; }
      else if (p == "axis")            { tableCell.setAxis(str); return; }
      else if (p == "bgColor")         { tableCell.setBgColor(str); return; }
      else if (p == "ch")              { tableCell.setCh(str); return; }
      else if (p == "chOff")           { tableCell.setChOff(str); return; }
      else if (p == "colSpan")         { tableCell.setColSpan(v.toInteger(exec)); return; }
      else if (p == "headers")         { tableCell.setHeaders(str); return; }
      else if (p == "height")          { tableCell.setHeight(str); return; }
      else if (p == "noWrap")          { tableCell.setNoWrap(v.toBoolean(exec)); return; }
      else if (p == "rowSpan")         { tableCell.setRowSpan(v.toInteger(exec)); return; }
      else if (p == "scope")           { tableCell.setScope(str); return; }
      else if (p == "vAlign")          { tableCell.setVAlign(str); return; }
      else if (p == "width")           { tableCell.setWidth(str); return; }
    }
    break;
    case ID_FRAMESET: {
      DOM::HTMLFrameSetElement frameSet = element;
      if      (p == "cols")            { frameSet.setCols(str); return; }
      else if (p == "rows")            { frameSet.setRows(str); return; }
    }
    break;
    case ID_FRAME: {
      DOM::HTMLFrameElement frameElement = element;
      if (p == "frameBorder")          { frameElement.setFrameBorder(str); return; }
      else if (p == "longDesc")        { frameElement.setLongDesc(str); return; }
      else if (p == "marginHeight")    { frameElement.setMarginHeight(str); return; }
      else if (p == "marginWidth")     { frameElement.setMarginWidth(str); return; }
      else if (p == "name")            { frameElement.setName(str); return; }
      else if (p == "noResize")        { frameElement.setNoResize(v.toBoolean(exec)); return; }
      else if (p == "scrolling")       { frameElement.setScrolling(str); return; }
      else if (p == "src")             { frameElement.setSrc(str); return; }
//      else if (p == "contentDocument") // new for DOM2 - not yet in khtml
//        return getDOMNode(exec,frameElement.contentDocument()); // type Document
    }
    break;
    case ID_IFRAME: {
      DOM::HTMLIFrameElement iFrame = element;
      if (p == "align")                { iFrame.setAlign(str); return; }
      else if (p == "frameBorder")     { iFrame.setFrameBorder(str); return; }
      else if (p == "height")          { iFrame.setHeight(str); return; }
      else if (p == "longDesc")        { iFrame.setLongDesc(str); return; }
      else if (p == "marginHeight")    { iFrame.setMarginHeight(str); return; }
      else if (p == "marginWidth")     { iFrame.setMarginWidth(str); return; }
      else if (p == "name")            { iFrame.setName(str); return; }
      else if (p == "scrolling")       { iFrame.setScrolling(str); return; }
      else if (p == "src")             { iFrame.setSrc(str); return; }
      else if (p == "width")           { iFrame.setWidth(str); return; }
//      else if (p == "contentDocument") // new for DOM2 - not yet in khtml
//        return getDOMNode(exec,iFrame.contentDocument); // type Document
    }
    break;
  }

  // generic properties
  if (p == "id")
    element.setId(str);
  else if (p == "title")
    element.setTitle(str);
  else if (p == "lang")
    element.setLang(str);
  else if (p == "dir")
    element.setDir(str);
  else if (p == "className")
    element.setClassName(str);
  else if ( p == "innerHTML")
    element.setInnerHTML(str);
  else if ( p == "innerText")
    element.setInnerText(str);
  else
    DOMElement::tryPut(exec,p,v,attr);
}

// -------------------------------------------------------------------------
/* Source for HMTLCollectionProtoTable. Use "make hashtables" to regenerate.
@begin HTMLCollectionProtoTable 3
  item		HTMLCollection::Item		DontDelete|Function 1
  namedItem	HTMLCollection::NamedItem	DontDelete|Function 1
  tags		HTMLCollection::Tags		DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("HTMLCollection", HTMLCollectionProto)
IMPLEMENT_PROTOFUNC(HTMLCollectionProtoFunc)
IMPLEMENT_PROTOTYPE(HTMLCollectionProto,HTMLCollectionProtoFunc)

//const ClassInfo HTMLCollection::info = { "HTMLCollection", 0, 0, 0 };

HTMLCollection::HTMLCollection(ExecState *exec, DOM::HTMLCollection c)
  : DOMObject(HTMLCollectionProto::self(exec)), collection(c) { }

HTMLCollection::~HTMLCollection()
{
  htmlCollections.remove(collection.handle());
}

// We have to implement hasProperty since we don't use a hashtable for 'selectedIndex' and 'length'
// ## this breaks "for (..in..)" though.
bool KJS::HTMLCollection::hasProperty(ExecState *exec, const UString &p, bool recursive) const
{
  if (p == "selectedIndex" || p == "length")
    return true;
  return DOMObject::hasProperty(exec,p,recursive);
}

Value KJS::HTMLCollection::tryGet(ExecState *exec, const UString &propertyName) const
{
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

    DOM::Node node;
    DOM::HTMLElement element;

    // name or index ?
    bool ok;
    unsigned int u = propertyName.toULong(&ok);
    if (ok)
      node = collection.item(u);
    else
      node = collection.namedItem(propertyName.string());

    element = node;
    return getDOMNode(exec,element);
  }
}

Value KJS::HTMLCollectionProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::HTMLCollection coll = static_cast<HTMLCollection *>(thisObj.imp())->toCollection();

  switch (id) {
  case KJS::HTMLCollection::Item:
    return getDOMNode(exec,coll.item(args[0].toUInt32(exec)));
  case KJS::HTMLCollection::Tags:
  {
    DOM::HTMLElement e = coll.base();
    return getDOMNodeList(exec, e.getElementsByTagName(args[0].toString(exec).string()));
  }
  case KJS::HTMLCollection::NamedItem:
    return getDOMNode(exec,coll.namedItem(args[0].toString(exec).string()));
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

DOM::Element KJS::HTMLSelectCollection::dummyElement()
{
  DOM::Document doc = element.ownerDocument();
  DOM::Element dummy = doc.createElement("OPTION");
  // Add an empty textnode inside, so that the text can be set from Javascript.
  DOM::Text text = doc.createTextNode( "" );
  dummy.appendChild( text );
  return dummy;
}

void KJS::HTMLSelectCollection::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int)
{
#ifdef KJS_VERBOSE
  kdDebug() << "KJS::HTMLSelectCollection::tryPut " << propertyName.qstring() << endl;
#endif
  // resize ?
  if (propertyName == "length") {
    long newLen = value.toInteger(exec);
    long diff = element.length() - newLen;

    if (diff < 0) { // add dummy elements
      do {
        element.add(dummyElement(), DOM::HTMLElement());
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
      element.add(dummyElement(), before);
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

Value Image::getValue(ExecState *, int token) const
{
  switch (token) {
  case Src:
    return String(src);
  case Complete:
    return Boolean(!img || img->status() >= khtml::CachedObject::Persistent);
  default:
    kdWarning() << "Image::getValue unhandled token " << token << endl;
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
  HTMLCollection *ret;
  if (c.isNull())
    return Null();
  else if ((ret = htmlCollections[c.handle()]))
    return ret;
  else {
    ret = new HTMLCollection(exec,c);
    htmlCollections.insert(c.handle(),ret);
    return ret;
  }
}

Value KJS::getSelectHTMLCollection(ExecState *exec, DOM::HTMLCollection c, DOM::HTMLSelectElement e)
{
  HTMLCollection *ret;
  if (c.isNull())
    return Null();
  else if ((ret = htmlCollections[c.handle()]))
    return ret;
  else {
    ret = new HTMLSelectCollection(exec, c, e);
    htmlCollections.insert(c.handle(),ret);
    return ret;
  }
}
