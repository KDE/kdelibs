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

#ifndef _HTML_OBJECT_H_
#define _HTML_OBJECT_H_

#include <html_document.h>
#include <html_base.h>
#include <html_misc.h>

#include "global.h"
#include "object.h"

class HTMLElement;
class HTMLCollection;

namespace KJS {

  class HTMLDocFunction : public KJSInternalFunction {
  public:
    HTMLDocFunction(DOM::HTMLDocument d, int i) : doc(d), id(i) { };
    virtual KJSO *get(const CString &p) const;
    KJSO *execute(KJSContext *);
    enum { Images, Applets, Links, Forms, Anchors, Open, Close,
	   Write, Writeln, ElByID, ElByName };
  private:
    DOM::HTMLDocument doc;
    int id;
  };

  class HTMLDocument : public HostObject {
  public:
    HTMLDocument(DOM::HTMLDocument d) : doc(d) { }
    virtual KJSO *get(const CString &p) const;
    virtual void put(const CString &p, KJSO *v, int attr = None);
  private:
    DOM::HTMLDocument doc;
  };

  class HTMLElement : public HostObject {
  public:
    HTMLElement(DOM::HTMLElement e) : element(e) { }
    virtual KJSO *get(const CString &p) const;
    virtual void put(const CString &p, KJSO *v, int attr = None);
  private:
    DOM::HTMLElement element;
  };

  class HTMLCollection : public HostObject {
  public:
    HTMLCollection(DOM::HTMLCollection c) : collection(c) { }
    virtual KJSO *get(const CString &p) const;
  private:
    DOM::HTMLCollection collection;
  };

  class HTMLCollectionFunc : public KJSInternalFunction {
  public:
    HTMLCollectionFunc(DOM::HTMLCollection c, int i) : coll(c), id(i) { };
    KJSO *execute(KJSContext *);
    enum { Item, NamedItem };
  private:
    DOM::HTMLCollection coll;
    int id;
  };

}; // namespace

#endif
