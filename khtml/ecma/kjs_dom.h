/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
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

#ifndef _DOM_CORE_H_
#define _DOM_CORE_H_

#include <iostream.h>

#include <dom_node.h>
#include <dom_doc.h>
#include <dom_element.h>

#include "global.h"
#include "object.h"

namespace KJS {

  class DOMNode : public HostObject {
  public:
    DOMNode(DOM::Node n) : node(n) { }
    virtual KJSO *get(const CString &p) const;
  private:
    DOM::Node node;
  };

  class DOMNodeList : public HostObject {
  public:
    DOMNodeList(DOM::NodeList l) : list(l) { }
    virtual KJSO *get(const CString &p) const;
  private:
    DOM::NodeList list;
  };

  class DOMDocument : public HostObject {
  public:
    DOMDocument(DOM::Document d) : doc(d) { }
    virtual KJSO *get(const CString &p) const;
  private:
    DOM::Document doc;
  };

  class DOMAttr : public HostObject {
  public:
    DOMAttr(DOM::Attr a) : attr(a) { }
  private:
    DOM::Attr attr;
  };

  class DOMElement : public HostObject {
  public:
    DOMElement(DOM::Element e) : element(e) { }
  private:
    DOM::Element element;
  };

}; // namespace

#endif
