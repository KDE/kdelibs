/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
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

#include <khtml.h>
#include <html_document.h>
#include <dom_string.h>

#include "kjs.h"
#include "operations.h"
#include "html_object.h"

using namespace KJS;

/* TODO: make these a shallow copies */
UString::UString(const DOM::DOMString &d)
{
  l = d.length();
  s = new UnicodeChar[l];
  memcpy(s, d.unicode(), l * sizeof(UnicodeChar));
}

DOM::DOMString UString::string() const
{
  return DOM::DOMString((QChar*) s, l);
}

HTMLDocument::HTMLDocument(KHTMLWidget *w, int id)
  : htmlw(w)
{
  const int attr = DontEnum | DontDelete | ReadOnly;

  if (!htmlw)
    return;

  switch (id) {
  case IDDocument:
    put("URL", new HTMLDocument(htmlw, IDURL), attr, true);
    put("title", new HTMLDocument(htmlw, IDTitle), attr, true);
    put("domain", new HTMLDocument(htmlw, IDDomain), attr, true);
    put("write", new HTMLDocFunction(htmlw, IDDocWrite), attr, true);
    break;
  case IDURL:
    put("toString", new HTMLDocFunction(htmlw, IDURL2S), attr, true);
    break;
  case IDTitle:
    put("toString", new HTMLDocFunction(htmlw, IDTitle2S), attr, true);
    break;
  case IDDomain:
    put("toString", new HTMLDocFunction(htmlw, IDDomain2S), attr, true);
    break;
  }
}

KJSO *HTMLDocFunction::execute(KJSContext *context)
{
  KJSO *result;
  Ptr v, n;
  DOM::HTMLDocument doc = htmlw->htmlDocument();
  
  switch (id) {
  case IDDocWrite:
    v = context->activation->get("0");
    n = toString(v);
    doc.write(n->sVal().string());
    result = new KJSUndefined();
    break;
  case IDURL2S:
    result = new KJSString(doc.URL());
    break;
  case IDTitle2S:
    result = new KJSString(doc.title());
    break;
  case IDDomain2S:
    result = new KJSString(doc.domain());
    break;
  default:
    assert((result = 0L));
  }

  return new KJSCompletion(Normal, result);
}
