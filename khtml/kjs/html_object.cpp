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

#include "khtml.h"
#include "html_document.h"
#include "html_element.h"
#include "html_misc.h"
#include "dom_string.h"

#include "kjs.h"
#include "operations.h"
#include "html_object.h"

// initialize HTML module
extern "C" {
  int kjs_html_init(void *arg)
  {
    KJS::KJSGlobal *global = KJScript::global();
    KHTMLWidget *w = (KHTMLWidget*) arg;
    global->put("document", new KJS::HTMLDocument(w), KJS::DontEnum, true);

    return 0;
  }
};

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

KJSO *KJS::HTMLDocFunction::execute(KJSContext *context)
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
  default:
    assert((result = 0L));
  }

  return new KJSCompletion(Normal, result);
}

KJSO *KJS::HTMLDocument::get(const CString &p) const
{
  DOM::HTMLDocument doc = htmlw->htmlDocument();
  KJSO *result;

  if (p == "title")
    result = new KJSString(doc.title());
  else if (p == "domain")
    result = new KJSString(doc.domain());
  else if (p == "URL")
    result = new KJSString(doc.URL());
  else if (p == "body")
    result = new HTMLElement(doc.body());
  else if (p == "images")
    result = new HTMLCollection(doc.images());
  else if (p == "links")
    result = new HTMLCollection(doc.links());
  else if (p == "write")
    result = new HTMLDocFunction(htmlw, IDDocWrite);
  else {
    fprintf(stderr, "Doc::get('%s') [undefined]\n", p.ascii());
    result = new KJSUndefined();
  }

  return result;
}

KJSO *KJS::HTMLElement::get(const CString &p) const
{
  if (p == "valueOf")
    return new KJSUndefined();

  return new KJSUndefined();
}

KJSO *KJS::HTMLCollection::get(const CString &p) const
{
  if (p == "valueOf")
    return new KJSUndefined();

  if (p == "length")
    return new KJSNumber(collection.length());

  return new KJSUndefined();
}
