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

#include <stdio.h>

#include <dom/html_base.h>
#include <dom/html_block.h>
#include <dom/html_document.h>
#include <dom/html_element.h>
#include <dom/html_form.h>
#include <dom/html_head.h>
#include <dom/html_image.h>
#include <dom/html_inline.h>
#include <dom/html_list.h>
#include <dom/html_misc.h>
#include <dom/html_table.h>
#include <dom/html_object.h>
#include <dom_string.h>

#include <kjs/operations.h>
#include "kjs_dom.h"
#include "kjs_html.h"

#include <htmltags.h>

using namespace KJS;

KJSO KJS::HTMLDocFunction::tryGet(const UString &p) const
{
  DOM::HTMLCollection coll;

  switch (id) {
  case Images:
    coll = doc.images();
    break;
  case Applets:
    coll = doc.applets();
    break;
  case Links:
    coll = doc.links();
    break;
  case Forms:
    coll = doc.forms();
    break;
  case Anchors:
    coll = doc.anchors();
    break;
  default:
    return Undefined();
  }

  KJSO tmp(new KJS::HTMLCollection(coll));

  return tmp.get(p);
}

Completion KJS::HTMLDocFunction::tryExecute(const List &args)
{
  KJSO result;
  String s;
  DOM::HTMLElement element;
  DOM::HTMLCollection coll;

  KJSO v = args[0];

  switch (id) {
  case Images:
    coll = doc.images();
    break;
  case Applets:
    coll = doc.applets();
    break;
  case Links:
    coll = doc.links();
    break;
  case Forms:
    coll = doc.forms();
    break;
  case Anchors:
    coll = doc.anchors();
    break;
  case Open:
    doc.open();
    result = Undefined();
    break;
  case Close:
    doc.close();
    result = Undefined();
    break;
  case Write:
    s = v.toString();
    doc.write(s.value().string());
    result = Undefined();
    break;
  case WriteLn:
    s = v.toString();
    doc.write((s.value() + "\n").string());
    result = Undefined();
    break;
  case GetElementById:
    s = v.toString();
    result = new DOMElement(doc.getElementById(s.value().string()));
    break;
  case GetElementsByName:
    s = v.toString();
    result = new DOMNodeList(doc.getElementsByName(s.value().string()));
    break;
  }

  // retrieve n'th element of collection
  if (id == Images || id == Applets || id == Links ||
      id == Forms || id == Anchors) {
    element = coll.item((unsigned long)v.toNumber().value());
    result = new HTMLElement(element);
  }

  return Completion(Normal, result);
}

const TypeInfo KJS::HTMLDocument::info = { "HTMLDocument", HostType,
					   &DOMDocument::info, 0, 0 };

KJSO KJS::HTMLDocument::tryGet(const UString &p) const
{
  KJSO result;

  if (p == "title")
    result = String(doc.title());
  else if (p == "referrer")
    result = String(doc.referrer());
  else if (p == "domain")
    result = String(doc.domain());
  else if (p == "URL")
    result = String(doc.URL());
  else if (p == "body")
    result = new HTMLElement(doc.body());
  else if (p == "images")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Images);
  else if (p == "applets")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Applets);
  else if (p == "links")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Links);
  else if (p == "forms")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Forms);
  else if (p == "anchors")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Anchors);
  else if (p == "cookie")
    result = String(doc.cookie());
  else if (p == "open")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Open);
  else if (p == "close")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Close);
  else if (p == "write")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Write);
  else if (p == "writeln")
    result = new HTMLDocFunction(doc, HTMLDocFunction::WriteLn);
  else if (p == "getElementById")
    result = new HTMLDocFunction(doc, HTMLDocFunction::GetElementById);
  else if (p == "getElementsByName")
    result = new HTMLDocFunction(doc, HTMLDocFunction::GetElementsByName);
  else {
    // look in base class (Document)
    KJSO tmp(new DOMDocument(doc));
    result = tmp.get(p);

    if (result.isA(UndefinedType)) {
      DOM::HTMLCollection coll = doc.images(); /* TODO: all() */
      DOM::HTMLElement element = coll.namedItem(p.string());
      result = new HTMLElement(element);
    }
  }

  return result;
}

void KJS::HTMLDocument::tryPut(const UString &p, const KJSO& v)
{
  if (p == "title")
    doc.setTitle(v.toString().value().string());
// ###  else if (p == "body") // body is supposed to by writable but it is not in khtml
//    doc.setBody(DOMNode(KJS::toNode(v)).toNode());
  else if (p == "cookie")
    doc.setCookie(v.toString().value().string());
  else {
    KJSO tmp(new DOMDocument(doc));
    tmp.put(p,v);
  }
}

const TypeInfo KJS::HTMLElement::info = { "HTMLElement", HostType,
					  &DOMElement::info, 0, 0 };

KJSO KJS::HTMLElement::tryGet(const UString &p) const
{
  DOM::DOMString str;

  switch (element.elementId()) {
    case ID_HTML: {
      DOM::HTMLHtmlElement html = element;
      if      (p == "version")         return String(html.version());
    }
    break;
    case ID_HEAD: {
      DOM::HTMLHeadElement head = element;
      if      (p == "profile")         return String(head.profile());
    }
    break;
    case ID_LINK: {
      DOM::HTMLLinkElement link = element;
      if      (p == "disabled")        return Boolean(link.disabled());
      else if (p == "charset")         return String(link.charset());
      else if (p == "href")            return String(link.href());
      else if (p == "hreflang")        return String(link.hreflang());
      else if (p == "media")           return String(link.media());
      else if (p == "rel")             return String(link.rel());
      else if (p == "rev")             return String(link.rev());
      else if (p == "target")          return String(link.target());
      else if (p == "type")            return String(link.type());
    }
    break;
    case ID_TITLE: {
      DOM::HTMLTitleElement title = element;
      if (p == "text")                 return String(title.text());
    }
    break;
    case ID_META: {
      DOM::HTMLMetaElement meta = element;
      if      (p == "content")         return String(meta.content());
      else if (p == "httpEquiv")       return String(meta.httpEquiv());
      else if (p == "name")            return String(meta.name());
      else if (p == "scheme")          return String(meta.scheme());
    }
    break;
    case ID_BASE: {
      DOM::HTMLBaseElement base = element;
      if      (p == "href")            return String(base.href());
      else if (p == "target")          return String(base.target());
    }
    break;
    case ID_ISINDEX: {
      DOM::HTMLIsIndexElement isindex = element;
      if      (p == "form")            return DOMNode::getDOMNode(isindex.form()); // type HTMLFormElement
      else if (p == "prompt")          return String(isindex.prompt());
    }
    break;
    case ID_STYLE: {
      DOM::HTMLStyleElement style = element;
      if      (p == "disabled")        return Boolean(style.disabled());
      else if (p == "media")           return String(style.media());
      else if (p == "type")            return String(style.type());
    }
    break;
    case ID_BODY: {
      DOM::HTMLBodyElement body = element;
      if      (p == "aLink")           return String(body.aLink());
      else if (p == "background")      return String(body.background());
      else if (p == "bgColor")         return String(body.bgColor());
      else if (p == "link")            return String(body.link());
      else if (p == "text")            return String(body.text());
      else if (p == "vLink")           return String(body.vLink());
    }
    break;
    case ID_FORM: {
      DOM::HTMLFormElement form = element;
      if      (p == "elements")        return new HTMLCollection(form.elements()); // type HTMLCollection
      else if (p == "length")          return Number((unsigned long)form.length());
      else if (p == "name")            return String(form.name());
      else if (p == "acceptCharset")   return String(form.acceptCharset());
      else if (p == "action")          return String(form.action());
      else if (p == "enctype")         return String(form.enctype());
      else if (p == "method")          return String(form.method());
      else if (p == "target")          return String(form.target());
      // methods
      else if (p == "submit")          return new HTMLElementFunction(element,HTMLElementFunction::Submit);
      else if (p == "reset")           return new HTMLElementFunction(element,HTMLElementFunction::Reset);
    }
    break;
    case ID_SELECT: {
      DOM::HTMLSelectElement select = element;
      if      (p == "type")            return String(select.type());
      else if (p == "selectedIndex")   return Number((unsigned long)select.selectedIndex());
      else if (p == "value")           return String(select.value());
      else if (p == "length")          return Number((unsigned long)select.length());
      else if (p == "form")            return DOMNode::getDOMNode(select.form()); // type HTMLFormElement
      else if (p == "options")         return new HTMLCollection(select.options()); // type HTMLCollection
      else if (p == "disabled")        return Boolean(select.disabled());
      else if (p == "multiple")        return Boolean(select.multiple());
      else if (p == "name")            return String(select.name());
      else if (p == "size")            return Number((unsigned long)select.size());
      else if (p == "tabIndex")        return Number((unsigned long)select.tabIndex());
      // methods
      else if (p == "add")             return new HTMLElementFunction(element,HTMLElementFunction::Add);
      else if (p == "remove")          return new HTMLElementFunction(element,HTMLElementFunction::Remove);
      else if (p == "blur")            return new HTMLElementFunction(element,HTMLElementFunction::Blur);
      else if (p == "focus")           return new HTMLElementFunction(element,HTMLElementFunction::Focus);
    }
    break;
    case ID_OPTGROUP: {
      DOM::HTMLOptGroupElement optgroup = element;
      if      (p == "disabled")        return Boolean(optgroup.disabled());
      else if (p == "label")           return String(optgroup.label());
    }
    break;
    case ID_OPTION: {
      DOM::HTMLOptionElement option = element;
      if      (p == "form")            return DOMNode::getDOMNode(option.form()); // type HTMLFormElement
      else if (p == "defaultSelected") return Boolean(option.defaultSelected());
      else if (p == "text")            return String(option.text());
      else if (p == "index")           return Number((unsigned long)option.index());
      else if (p == "disabled")        return Boolean(option.disabled());
      else if (p == "label")           return String(option.label());
      else if (p == "selected")        return Boolean(option.selected());
      else if (p == "value")           return String(option.value());
    }
    break;
    case ID_INPUT: {
      DOM::HTMLInputElement input = element;
      if      (p == "defaultValue")    return String(input.defaultValue());
      else if (p == "defaultChecked")  return Boolean(input.defaultChecked());
      else if (p == "form")            return DOMNode::getDOMNode(input.form()); // type HTMLFormElement
      else if (p == "accept")          return String(input.accept());
      else if (p == "accessKey")       return String(input.accessKey());
      else if (p == "align")           return String(input.align());
      else if (p == "alt")             return String(input.alt());
      else if (p == "checked")         return Boolean(input.checked());
      else if (p == "disabled")        return Boolean(input.disabled());
      else if (p == "maxLength")       return Number((unsigned long)input.maxLength());
      else if (p == "name")            return String(input.name());
      else if (p == "readOnly")        return Boolean(input.readOnly());
      else if (p == "size")            return String(input.size());
      else if (p == "src")             return String(input.src());
      else if (p == "tabIndex")        return Number((unsigned long)input.tabIndex());
      else if (p == "type")            return String(input.type());
      else if (p == "useMap")          return String(input.useMap());
      else if (p == "value")           return String(input.value());
      // methods
      else if (p == "blur")            return new HTMLElementFunction(element,HTMLElementFunction::Blur);
      else if (p == "focus")           return new HTMLElementFunction(element,HTMLElementFunction::Focus);
      else if (p == "select")          return new HTMLElementFunction(element,HTMLElementFunction::Select);
      else if (p == "click")           return new HTMLElementFunction(element,HTMLElementFunction::Click);
    }
    break;
    case ID_TEXTAREA: {
      DOM::HTMLTextAreaElement textarea = element;
      if      (p == "defaultValue")    return String(textarea.defaultValue());
      else if (p == "form")            return DOMNode::getDOMNode(textarea.form()); // type HTMLFormElement
      else if (p == "accessKey")       return String(textarea.accessKey());
      else if (p == "cols")            return Number((unsigned long)textarea.cols());
      else if (p == "disabled")        return Boolean(textarea.disabled());
      else if (p == "name")            return String(textarea.name());
      else if (p == "readOnly")        return Boolean(textarea.readOnly());
      else if (p == "rows")            return Number((unsigned long)textarea.rows());
      else if (p == "tabIndex")        return Number((unsigned long)textarea.tabIndex());
      else if (p == "type")            return String(textarea.type());
      else if (p == "value")           return String(textarea.value());
      // methods
      else if (p == "blur")            return new HTMLElementFunction(element,HTMLElementFunction::Blur);
      else if (p == "focus")           return new HTMLElementFunction(element,HTMLElementFunction::Focus);
      else if (p == "select")          return new HTMLElementFunction(element,HTMLElementFunction::Select);
    }
    break;
    case ID_BUTTON: {
      DOM::HTMLButtonElement button = element;
      if      (p == "form")            return DOMNode::getDOMNode(button.form()); // type HTMLFormElement
      else if (p == "accessKey")       return String(button.accessKey());
      else if (p == "disabled")        return Boolean(button.disabled());
      else if (p == "name")            return String(button.name());
      else if (p == "tabIndex")        return Number((unsigned long)button.tabIndex());
      else if (p == "type")            return String(button.type());
      else if (p == "value")           return String(button.value());
    }
    break;
    case ID_LABEL: {
      DOM::HTMLLabelElement label = element;
      if      (p == "form")            return DOMNode::getDOMNode(label.form()); // type HTMLFormElement
      else if (p == "accessKey")       return String(label.accessKey());
      else if (p == "htmlFor")         return String(label.htmlFor());
    }
    break;
    case ID_FIELDSET: {
      DOM::HTMLFieldSetElement fieldSet = element;
      if      (p == "form")            return DOMNode::getDOMNode(fieldSet.form()); // type HTMLFormElement
    }
    break;
    case ID_LEGEND: {
      DOM::HTMLLegendElement legend = element;
      if      (p == "form")            return DOMNode::getDOMNode(legend.form()); // type HTMLFormElement
      else if (p == "accessKey")       return String(legend.accessKey());
      else if (p == "align")           return String(legend.align());
    }
    break;
    case ID_UL: {
      DOM::HTMLUListElement uList = element;
      if      (p == "compact")         return Boolean(uList.compact());
      else if (p == "type")            return String(uList.type());
    }
    break;
    case ID_OL: {
      DOM::HTMLOListElement oList = element;
      if      (p == "compact")         return Boolean(oList.compact());
      else if (p == "start")           return Number((unsigned long)oList.start());
      else if (p == "type")            return String(oList.type());
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
      if      (p == "type")            return String(li.type());
      else if (p == "value")           return Number((unsigned long)li.value());
    }
    break;
    case ID_DIV: {
      DOM::HTMLDivElement div = element;
      if      (p == "align")           return String(div.align());
    }
    break;
    case ID_P: {
      DOM::HTMLParagraphElement paragraph = element;
      if      (p == "align")           return String(paragraph.align());
    }
    break;
    case ID_H1: { // ### H2, H3 ,H4 ,H5 ,H6
      DOM::HTMLHeadingElement heading = element;
      if      (p == "align")           return String(heading.align());
    }
    break;
    case ID_BLOCKQUOTE:
    case ID_Q: {
      DOM::HTMLQuoteElement quote = element;
      if      (p == "cite")            return String(quote.cite());
    }
    case ID_PRE: {
      DOM::HTMLPreElement pre = element;
      if      (p == "width")           return Number((unsigned long)pre.width());
    }
    break;
    case ID_BR: {
      DOM::HTMLBRElement br = element;
      if      (p == "clear")           return String(br.clear());
    }
    break;
    case ID_BASEFONT: {
      DOM::HTMLBaseFontElement baseFont = element;
      if      (p == "color")           return String(baseFont.color());
      else if (p == "face")            return String(baseFont.face());
      else if (p == "size")            return String(baseFont.size());
    }
    break;
    case ID_FONT: {
      DOM::HTMLFontElement font = element;
      if      (p == "color")           return String(font.color());
      else if (p == "face")            return String(font.face());
      else if (p == "size")            return String(font.size());
    }
    break;
    case ID_HR: {
      DOM::HTMLHRElement hr = element;
      if      (p == "align")           return String(hr.align());
      else if (p == "noShade")         return Boolean(hr.noShade());
      else if (p == "size")            return String(hr.size());
      else if (p == "width")           return String(hr.width());
    }
    break;
    case ID_INS:
    case ID_DEL: {
      DOM::HTMLModElement mod = element;
      if      (p == "cite")            return String(mod.cite());
      else if (p == "dateTime")        return String(mod.dateTime());
    }
    break;
    case ID_A: {
      DOM::HTMLAnchorElement anchor = element;
      if      (p == "accessKey")       return String(anchor.accessKey());
      else if (p == "charset")         return String(anchor.charset());
      else if (p == "coords")          return String(anchor.coords());
      else if (p == "href")            return String(anchor.href());
      else if (p == "hreflang")        return String(anchor.hreflang());
      else if (p == "name")            return String(anchor.name());
      else if (p == "rel")             return String(anchor.rel());
      else if (p == "rev")             return String(anchor.rev());
      else if (p == "shape")           return String(anchor.shape());
      else if (p == "tabIndex")        return Number((unsigned long)anchor.tabIndex());
      else if (p == "target")          return String(anchor.target());
      else if (p == "type")            return String(anchor.type());
      // methods
      else if (p == "blur")            return new HTMLElementFunction(element,HTMLElementFunction::Blur);
      else if (p == "focus")           return new HTMLElementFunction(element,HTMLElementFunction::Focus);
    }
    break;
    case ID_IMG: {
      DOM::HTMLImageElement image = element;
      if      (p == "lowSrc")          return String(image.lowSrc());
      else if (p == "name")            return String(image.name());
      else if (p == "align")           return String(image.align());
      else if (p == "alt")             return String(image.alt());
      else if (p == "border")          return String(image.border());
      else if (p == "height")          return String(image.height());
      else if (p == "hspace")          return String(image.hspace());
      else if (p == "isMap")           return Boolean(image.isMap());
      else if (p == "longDesc")        return String(image.longDesc());
      else if (p == "src")             return String(image.src());
      else if (p == "useMap")          return String(image.useMap());
      else if (p == "vspace")          return String(image.vspace());
      else if (p == "width")           return String(image.width());
    }
    break;
    case ID_OBJECT: {
      DOM::HTMLObjectElement object = element;
      if      (p == "form")            return DOMNode::getDOMNode(object.form()); // type HTMLFormElement
      else if (p == "code")            return String(object.code());
      else if (p == "align")           return String(object.align());
      else if (p == "archive")         return String(object.archive());
      else if (p == "border")          return String(object.border());
      else if (p == "codeBase")        return String(object.codeBase());
      else if (p == "codeType")        return String(object.codeType());
      else if (p == "data")            return String(object.data());
      else if (p == "declare")         return Boolean(object.declare());
      else if (p == "height")          return String(object.height());
      else if (p == "hspace")          return String(object.hspace());
      else if (p == "name")            return String(object.name());
      else if (p == "standby")         return String(object.standby());
      else if (p == "tabIndex")        return Number((unsigned long)object.tabIndex());
      else if (p == "type")            return String(object.type());
      else if (p == "useMap")          return String(object.useMap());
      else if (p == "vspace")          return String(object.vspace());
      else if (p == "width")           return String(object.width());
//      else if (p == "contentDocument") // new for DOM2 - not yet in khtml
//        return DOMNode::getDOMNode(object.contentDocument()); // type Document
    }
    break;
    case ID_PARAM: {
      DOM::HTMLParamElement param = element;
      if      (p == "name")            return String(param.name());
      else if (p == "type")            return String(param.type());
      else if (p == "value")           return String(param.value());
      else if (p == "valueType")       return String(param.valueType());
    }
    break;
    case ID_APPLET: {
      DOM::HTMLAppletElement applet = element;
      if      (p == "align")           return String(applet.align());
      else if (p == "alt")             return String(applet.alt());
      else if (p == "archive")         return String(applet.archive());
      else if (p == "code")            return String(applet.code());
      else if (p == "codeBase")        return String(applet.codeBase());
      else if (p == "height")          return String(applet.height());
      else if (p == "hspace")          return String(applet.hspace());
      else if (p == "name")            return String(applet.name());
      else if (p == "object")          return String(applet.object());
      else if (p == "vspace")          return String(applet.vspace());
      else if (p == "width")           return String(applet.width());
    }
    break;
    case ID_MAP: {
      DOM::HTMLMapElement map = element;
      if      (p == "areas")           return new HTMLCollection(map.areas()); // type HTMLCollection
      else if (p == "name")            return String(map.name());
    }
    break;
    case ID_AREA: {
      DOM::HTMLAreaElement area = element;
      if      (p == "accessKey")       return String(area.accessKey());
      else if (p == "alt")             return String(area.alt());
      else if (p == "coords")          return String(area.coords());
      else if (p == "href")            return String(area.href());
      else if (p == "noHref")          return Boolean(area.noHref());
      else if (p == "shape")           return String(area.shape());
      else if (p == "tabIndex")        return Number((unsigned long)area.tabIndex());
      else if (p == "target")          return String(area.target());
    }
    break;
    case ID_SCRIPT: {
      DOM::HTMLScriptElement script = element;
      if      (p == "text")            return String(script.text());
      else if (p == "htmlFor")         return String(script.htmlFor());
      else if (p == "event")           return String(script.event());
      else if (p == "charset")         return String(script.charset());
      else if (p == "defer")           return Boolean(script.defer());
      else if (p == "src")             return String(script.src());
      else if (p == "type")            return String(script.type());
    }
    break;
    case ID_TABLE: {
      DOM::HTMLTableElement table = element;
      if      (p == "caption")         return DOMNode::getDOMNode(table.caption()); // type HTMLTableCaptionElement
      else if (p == "tHead")           return DOMNode::getDOMNode(table.tHead()); // type HTMLTableSectionElement
      else if (p == "tFoot")           return DOMNode::getDOMNode(table.tFoot()); // type HTMLTableSectionElement
      else if (p == "rows")            return new HTMLCollection(table.rows()); // type HTMLCollection
      else if (p == "tBodies")         return new HTMLCollection(table.tBodies()); // type HTMLCollection
      else if (p == "align")           return String(table.align());
      else if (p == "bgColor")         return String(table.bgColor());
      else if (p == "border")          return String(table.border());
      else if (p == "cellPadding")     return String(table.cellPadding());
      else if (p == "cellSpacing")     return String(table.cellSpacing());
      else if (p == "frame")           return String(table.frame());
      else if (p == "rules")           return String(table.rules());
      else if (p == "summary")         return String(table.summary());
      else if (p == "width")           return String(table.width());
      // methods
      else if (p == "createTHead")     return new HTMLElementFunction(element,HTMLElementFunction::CreateTHead);
      else if (p == "deleteTHead")     return new HTMLElementFunction(element,HTMLElementFunction::DeleteTHead);
      else if (p == "createTFoot")     return new HTMLElementFunction(element,HTMLElementFunction::CreateTFoot);
      else if (p == "deleteTFoot")     return new HTMLElementFunction(element,HTMLElementFunction::DeleteTFoot);
      else if (p == "createCaption")   return new HTMLElementFunction(element,HTMLElementFunction::CreateCaption);
      else if (p == "deleteCaption")   return new HTMLElementFunction(element,HTMLElementFunction::DeleteCaption);
      else if (p == "insertRow")       return new HTMLElementFunction(element,HTMLElementFunction::InsertRow);
      else if (p == "deleteRow")       return new HTMLElementFunction(element,HTMLElementFunction::DeleteRow);
    }
    break;
    case ID_CAPTION: {
      DOM::HTMLTableCaptionElement tableCaption;
      if      (p == "align")           return String(tableCaption.align());
    }
    break;
    case ID_COL: {
      DOM::HTMLTableColElement tableCol = element;
      if      (p == "align")           return String(tableCol.align());
      else if (p == "ch")              return String(tableCol.ch());
      else if (p == "chOff")           return String(tableCol.chOff());
      else if (p == "span")            return Number((unsigned long)tableCol.span());
      else if (p == "vAlign")          return String(tableCol.vAlign());
      else if (p == "width")           return String(tableCol.width());
    }
    break;
    case ID_THEAD:
    case ID_TBODY:
    case ID_TFOOT: {
      DOM::HTMLTableSectionElement tableSection = element;
      if      (p == "align")           return String(tableSection.align());
      else if (p == "ch")              return String(tableSection.ch());
      else if (p == "chOff")           return String(tableSection.chOff());
      else if (p == "vAlign")          return String(tableSection.vAlign());
      else if (p == "rows")            return new HTMLCollection(tableSection.rows()); // type HTMLCollection
      // methods
      else if (p == "insertRow")       return new HTMLElementFunction(element,HTMLElementFunction::InsertRow);
      else if (p == "deleteRow")       return new HTMLElementFunction(element,HTMLElementFunction::DeleteRow);
    }
    break;
    case ID_TR: {
      DOM::HTMLTableRowElement tableRow = element;
      if      (p == "rowIndex")        return Number((unsigned long)tableRow.rowIndex());
      else if (p == "sectionRowIndex") return Number((unsigned long)tableRow.sectionRowIndex());
      else if (p == "cells")           return new HTMLCollection(tableRow.cells()); // type HTMLCollection
      else if (p == "align")           return String(tableRow.align());
      else if (p == "bgColor")         return String(tableRow.bgColor());
      else if (p == "ch")              return String(tableRow.ch());
      else if (p == "chOff")           return String(tableRow.chOff());
      else if (p == "vAlign")          return String(tableRow.vAlign());
      // methods
      else if (p == "insertCell")       return new HTMLElementFunction(element,HTMLElementFunction::InsertCell);
      else if (p == "deleteCell")       return new HTMLElementFunction(element,HTMLElementFunction::DeleteCell);
    }
    break;
    case ID_TH:
    case ID_TD: {
      DOM::HTMLTableCellElement tableCell = element;
      if      (p == "cellIndex")       return Number((unsigned long)tableCell.cellIndex());
      else if (p == "abbr")            return String(tableCell.abbr());
      else if (p == "align")           return String(tableCell.align());
      else if (p == "axis")            return String(tableCell.axis());
      else if (p == "bgColor")         return String(tableCell.bgColor());
      else if (p == "ch")              return String(tableCell.ch());
      else if (p == "chOff")           return String(tableCell.chOff());
      else if (p == "colSpan")         return Number((unsigned long)tableCell.colSpan());
      else if (p == "headers")         return String(tableCell.headers());
      else if (p == "height")          return String(tableCell.height());
      else if (p == "noWrap")          return Boolean(tableCell.noWrap());
      else if (p == "rowSpan")         return Number((unsigned long)tableCell.rowSpan());
      else if (p == "scope")           return String(tableCell.scope());
      else if (p == "vAlign")          return String(tableCell.vAlign());
      else if (p == "width")           return String(tableCell.width());
    }
    break;
    case ID_FRAMESET: {
      DOM::HTMLFrameSetElement frameSet = element;
      if      (p == "cols")            return String(frameSet.cols());
      else if (p == "rows")            return String(frameSet.rows());
    }
    break;
    case ID_FRAME: {
      DOM::HTMLFrameElement frameElement = element;
      if (p == "frameBorder")          return String(frameElement.frameBorder());
      else if (p == "longDesc")        return String(frameElement.longDesc());
      else if (p == "marginHeight")    return String(frameElement.marginHeight());
      else if (p == "marginWidth")     return String(frameElement.marginWidth());
      else if (p == "name")            return String(frameElement.name());
      else if (p == "noResize")        return Boolean(frameElement.noResize());
      else if (p == "scrolling")       return String(frameElement.scrolling());
      else if (p == "src")             return String(frameElement.src());
//      else if (p == "contentDocument") // new for DOM2 - not yet in khtml
//        return DOMNode::getDOMNode(frameElement.contentDocument()); // type Document
    }
    break;
    case ID_IFRAME: {
      DOM::HTMLIFrameElement iFrame = element;
      if (p == "align")                return String(iFrame.align());
      else if (p == "frameBorder")     return String(iFrame.frameBorder());
      else if (p == "height")          return String(iFrame.height());
      else if (p == "longDesc")        return String(iFrame.longDesc());
      else if (p == "marginHeight")    return String(iFrame.marginHeight());
      else if (p == "marginWidth")     return String(iFrame.marginWidth());
      else if (p == "name")            return String(iFrame.name());
      else if (p == "scrolling")       return String(iFrame.scrolling());
      else if (p == "src")             return String(iFrame.src());
      else if (p == "width")           return String(iFrame.width());
//      else if (p == "contentDocument") // new for DOM2 - not yet in khtml
//        return DOMNode::getDOMNode(iFrame.contentDocument); // type Document
    }
    break;
  }

  // generic properties
  if (p == "id")
    str = element.id();
  else if (p == "title")
    str = element.title();
  else if (p == "lang")
    str = element.lang();
  else if (p == "dir")
    str = element.dir();
  else if (p == "className")
    str = element.className();
  else {
    KJSO tmp(new DOMElement(element));
    return tmp.get(p);
  }

  return String(str);
}

Completion KJS::HTMLElementFunction::tryExecute(const List &args)
{
  KJSO result;

  switch (id) {
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
        select.add(DOMNode(KJS::toNode(args[0])).toNode(),DOMNode(KJS::toNode(args[1])).toNode());
        result = Undefined();
      }
      else if (id == Remove) {
        select.remove(args[0].toNumber().intValue());
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
        result = DOMNode::getDOMNode(table.createTHead());
      else if (id == DeleteTHead) {
        table.deleteTHead();
        result = Undefined();
      }
      else if (id == CreateTFoot)
        result = DOMNode::getDOMNode(table.createTFoot());
      else if (id == DeleteTFoot) {
        table.deleteTFoot();
        result = Undefined();
      }
      else if (id == CreateCaption)
        result = DOMNode::getDOMNode(table.createCaption());
      else if (id == DeleteCaption) {
        table.deleteCaption();
        result = Undefined();
      }
      else if (id == InsertRow)
        result = DOMNode::getDOMNode(table.insertRow(args[0].toNumber().intValue()));
      else if (id == DeleteRow) {
        table.deleteRow(args[0].toNumber().intValue());
        result = Undefined();
      }
    }
    break;
    case ID_THEAD:
    case ID_TBODY:
    case ID_TFOOT: {
      DOM::HTMLTableSectionElement tableSection = element;
      if (id == InsertRow)
        result = DOMNode::getDOMNode(tableSection.insertRow(args[0].toNumber().intValue()));
      else if (id == DeleteRow) {
        tableSection.deleteRow(args[0].toNumber().value());
        result = Undefined();
      }
    }
    break;
    case ID_TR: {
      DOM::HTMLTableRowElement tableRow = element;
      if (id == InsertCell)
        result = DOMNode::getDOMNode(tableRow.insertCell(args[0].toNumber().intValue()));
      else if (id == DeleteCell) {
        tableRow.deleteCell(args[0].toNumber().intValue());
        result = Undefined();
      }
    }
    break;
  }

  return Completion(Normal, result);
}

void KJS::HTMLElement::tryPut(const UString &p, const KJSO& v)
{
  DOM::DOMString str = v.toString().value().string();
  DOM::Node n = DOMNode(KJS::toNode(v)).toNode();

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
      if      (p == "disabled")        { link.setDisabled(v.toBoolean().value()); return; }
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
      if      (p == "disabled")        { style.setDisabled(v.toBoolean().value()); return; }
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
      else if (p == "action")          { form.setAction(str); return; }
      else if (p == "enctype")         { form.setEnctype(str); return; }
      else if (p == "method")          { form.setMethod(str); return; }
      else if (p == "target")          { form.setTarget(str); return; }
    }
    break;
    case ID_SELECT: {
      DOM::HTMLSelectElement select = element;
      // read-only: type
      if (p == "selectedIndex")        { select.setSelectedIndex(v.toNumber().intValue()); return; }
      else if (p == "value")           { select.setValue(str); return; }
      // read-only: length
      // read-only: form
      // read-only: options
      else if (p == "disabled")        { select.setDisabled(v.toBoolean().value()); return; }
      else if (p == "multiple")        { select.setMultiple(v.toBoolean().value()); return; }
      else if (p == "name")            { select.setName(str); return; }
      else if (p == "size")            { select.setSize(v.toNumber().intValue()); return; }
      else if (p == "tabIndex")        { select.setTabIndex(v.toNumber().intValue()); return; }
    }
    break;
    case ID_OPTGROUP: {
      DOM::HTMLOptGroupElement optgroup = element;
      if      (p == "disabled")        { optgroup.setDisabled(v.toBoolean().value()); return; }
      else if (p == "label")           { optgroup.setLabel(str); return; }
    }
    break;
    case ID_OPTION: {
      DOM::HTMLOptionElement option = element;
      // read-only: form
      if (p == "defaultSelected")      { option.setDefaultSelected(v.toBoolean().value()); return; }
      // read-only: text
      // read-only: index
      else if (p == "disabled")        { option.setDisabled(v.toBoolean().value()); return; }
      else if (p == "label")           { option.setLabel(str); return; }
      // ### selected is read-only in DOM1, and hence khtml at the moment
      // else if (p == "selected")        { option.setSelected(v.toBoolean().value()); return; }
      else if (p == "value")           { option.setValue(str); return; }
    }
    break;
    case ID_INPUT: {
      DOM::HTMLInputElement input = element;
      if      (p == "defaultValue")    { input.setDefaultValue(str); return; }
      else if (p == "defaultChecked")  { input.setDefaultChecked(v.toBoolean().value()); return; }
      // read-only: form
      else if (p == "accept")          { input.setAccept(str); return; }
      else if (p == "accessKey")       { input.setAccessKey(str); return; }
      else if (p == "align")           { input.setAlign(str); return; }
      else if (p == "alt")             { input.setAlt(str); return; }
      else if (p == "checked")         { input.setChecked(v.toBoolean().value()); return; }
      else if (p == "disabled")        { input.setDisabled(v.toBoolean().value()); return; }
      else if (p == "maxLength")       { input.setMaxLength(v.toNumber().intValue()); return; }
      else if (p == "name")            { input.setName(str); return; }
      else if (p == "readOnly")        { input.setReadOnly(v.toBoolean().value()); return; }
      else if (p == "size")            { input.setSize(str); return; }
      else if (p == "src")             { input.setSrc(str); return; }
      else if (p == "tabIndex")        { input.setTabIndex(v.toNumber().intValue()); return; }
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
      else if (p == "cols")            { textarea.setCols(v.toNumber().intValue()); return; }
      else if (p == "disabled")        { textarea.setDisabled(v.toBoolean().value()); return; }
      else if (p == "name")            { textarea.setName(str); return; }
      else if (p == "readOnly")        { textarea.setReadOnly(v.toBoolean().value()); return; }
      else if (p == "rows")            { textarea.setRows(v.toNumber().intValue()); return; }
      else if (p == "tabIndex")        { textarea.setTabIndex(v.toNumber().intValue()); return; }
      // read-only: type
      else if (p == "value")           { textarea.setValue(str); return; }
    }
    break;
    case ID_BUTTON: {
      DOM::HTMLButtonElement button = element;
      // read-only: form
      if (p == "accessKey")            { button.setAccessKey(str); return; }
      else if (p == "disabled")        { button.setDisabled(v.toBoolean().value()); return; }
      else if (p == "name")            { button.setName(str); return; }
      else if (p == "tabIndex")        { button.setTabIndex(v.toNumber().intValue()); return; }
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
      if      (p == "compact")         { uList.setCompact(v.toBoolean().value()); return; }
      else if (p == "type")            { uList.setType(str); return; }
    }
    break;
    case ID_OL: {
      DOM::HTMLOListElement oList = element;
      if      (p == "compact")         { oList.setCompact(v.toBoolean().value()); return; }
      else if (p == "start")           { oList.setStart(v.toNumber().intValue()); return; }
      else if (p == "type")            { oList.setType(str); return; }
    }
    break;
    case ID_DL: {
      DOM::HTMLDListElement dList = element;
      if      (p == "compact")         { dList.setCompact(v.toBoolean().value()); return; }
    }
    break;
    case ID_DIR: {
      DOM::HTMLDirectoryElement directory = element;
      if      (p == "compact")         { directory.setCompact(v.toBoolean().value()); return; }
    }
    break;
    case ID_MENU: {
      DOM::HTMLMenuElement menu = element;
      if      (p == "compact")         { menu.setCompact(v.toBoolean().value()); return; }
    }
    break;
    case ID_LI: {
      DOM::HTMLLIElement li = element;
      if      (p == "type")            { li.setType(str); return; }
      else if (p == "value")           { li.setValue(v.toNumber().intValue()); return; }
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
    case ID_BLOCKQUOTE:
    case ID_Q: {
      DOM::HTMLQuoteElement quote = element;
      if      (p == "cite")            { quote.setCite(str); return; }
    }
    break;
    case ID_PRE: {
      DOM::HTMLPreElement pre = element;
      if      (p == "width")           { pre.setWidth(v.toNumber().intValue()); return; }
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
      else if (p == "noShade")         { hr.setNoShade(v.toBoolean().value()); return; }
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
      else if (p == "tabIndex")        { anchor.setTabIndex(v.toNumber().intValue()); return; }
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
      else if (p == "isMap")           { image.setIsMap(v.toBoolean().value()); return; }
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
      else if (p == "declare")         { object.setDeclare(v.toBoolean().value()); return; }
      else if (p == "height")          { object.setHeight(str); return; }
      else if (p == "hspace")          { object.setHspace(str); return; }
      else if (p == "name")            { object.setName(str); return; }
      else if (p == "standby")         { object.setStandby(str); return; }
      else if (p == "tabIndex")        { object.setTabIndex(v.toNumber().intValue()); return; }
      else if (p == "type")            { object.setType(str); return; }
      else if (p == "useMap")          { object.setUseMap(str); return; }
      else if (p == "vspace")          { object.setVspace(str); return; }
      else if (p == "width")           { object.setWidth(str); return; }
//      else if (p == "contentDocument") // new for DOM2 - not yet in khtml
//        return DOMNode::getDOMNode(object.contentDocument()); // type Document
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
      else if (p == "noHref")          { area.setNoHref(v.toBoolean().value()); return; }
      else if (p == "shape")           { area.setShape(str); return; }
      else if (p == "tabIndex")        { area.setTabIndex(v.toNumber().intValue()); return; }
      else if (p == "target")          { area.setTarget(str); return; }
    }
    break;
    case ID_SCRIPT: {
      DOM::HTMLScriptElement script = element;
      if      (p == "text")            { script.setText(str); return; }
      else if (p == "htmlFor")         { script.setHtmlFor(str); return; }
      else if (p == "event")           { script.setEvent(str); return; }
      else if (p == "charset")         { script.setCharset(str); return; }
      else if (p == "defer")           { script.setDefer(v.toBoolean().value()); return; }
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
      else if (p == "span")            { tableCol.setSpan(v.toNumber().intValue()); return; }
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
      else if (p == "colSpan")         { tableCell.setColSpan(v.toNumber().intValue()); return; }
      else if (p == "headers")         { tableCell.setHeaders(str); return; }
      else if (p == "height")          { tableCell.setHeight(str); return; }
      else if (p == "noWrap")          { tableCell.setNoWrap(v.toBoolean().value()); return; }
      else if (p == "rowSpan")         { tableCell.setRowSpan(v.toNumber().intValue()); return; }
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
      else if (p == "noResize")        { frameElement.setNoResize(v.toBoolean().value()); return; }
      else if (p == "scrolling")       { frameElement.setScrolling(str); return; }
      else if (p == "src")             { frameElement.setSrc(str); return; }
//      else if (p == "contentDocument") // new for DOM2 - not yet in khtml
//        return DOMNode::getDOMNode(frameElement.contentDocument()); // type Document
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
//        return DOMNode::getDOMNode(iFrame.contentDocument); // type Document
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
  else {
    KJSO tmp(new DOMElement(element));
    tmp.put(p,v);
  }
}

KJSO KJS::HTMLCollection::tryGet(const UString &p) const
{
  KJSO result;

  if (p == "length")
    result = Number(collection.length());
  else if (p == "item")
    result = new HTMLCollectionFunc(collection, HTMLCollectionFunc::Item);
  else if (p == "namedItem")
    result = new HTMLCollectionFunc(collection, HTMLCollectionFunc::NamedItem);
  else {
    DOM::Node node;
    DOM::HTMLElement element;
    unsigned long u;

    // name or index ?
    int ret = sscanf(p.cstring().c_str(), "%lu", &u);
    if (ret)
      node = collection.item(u);
    else
      node = collection.namedItem(p.string());

    element = node;
    result = new HTMLElement(element);
  }

  return result;
}

Completion KJS::HTMLCollectionFunc::tryExecute(const List &args)
{
  KJSO result;

  assert(id == Item || id == NamedItem);

  switch (id) {
    case Item:
      result = DOMNode::getDOMNode(coll.item((unsigned long)args[0].toNumber().value()));
      break;
    case NamedItem:
      result = DOMNode::getDOMNode(coll.namedItem(args[0].toString().value().string()));
      break;
  }

  return Completion(Normal, result);
}

////////////////////// Image Object ////////////////////////

ImageObject::ImageObject(const Global& global)
{
  Constructor ctor(new ImageConstructor(global));
  setConstructor(ctor);
  setPrototype(global.objectPrototype());

  put("length", Number(2), DontEnum);
}

Completion ImageObject::tryExecute(const List &)
{
  return Completion(Normal, Undefined());
}

ImageConstructor::ImageConstructor(const Global& glob)
  : global(glob)
{
  setPrototype(global.functionPrototype());
}

Object ImageConstructor::construct(const List &)
{
  /* TODO: fetch optional height & width from arguments */

  Object result(new Image());
  /* TODO: do we need a prototype ? */

  return result;
}

KJSO Image::tryGet(const UString &p) const
{
  KJSO result;

  if (p == "src")
    result = String(src);
  else
    result = Undefined();

  return result;
}

void Image::tryPut(const UString &p, const KJSO& v)
{
  if (p == "src") {
    String str = v.toString();
    src = str.value();
  }
}
