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

#ifndef _KJS_BINDING_H_
#define _KJS_BINDING_H_

#include <kjs/object.h>
#include <dom/dom_node.h>

namespace KJS {

  /**
   * Abstract base class for DOM objects that derive from Node.
   * Derived classes must implement the toNode() method to allow
   * dynamic type casting.
   */
  class NodeObject : public HostObject {
  public:
    virtual DOM::Node toNode() const = 0L;
  };

  /**
   * Convert an object to a Node. Returns a null Node if not possible.
   */
  DOM::Node toNode(KJSO *);

}; // namespace

#endif
