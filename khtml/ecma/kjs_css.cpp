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

#include "kjs_css.h"

#include <xml/dom_nodeimpl.h>
#include <rendering/render_style.h>
#include <kjs/types.h>

using namespace KJS;

KJSO Style::get(const UString &p) const
{
  KJSO result;

  khtml::RenderStyle *style = node.handle()->style();

  // ### code below is just for testing. it's neither complete nor correct.
  
  if (p == "backgroundColor")
    result = String(style->backgroundColor().name());
  else if (p == "left")
    result = Number(style->left().value);
  else if (p == "top")
    result = Number(style->top().value);
  else
    result = Undefined();

  return result;
}

void Style::put(const UString &p, const KJSO& v)
{
  khtml::RenderStyle *style = node.handle()->style();

  if (p == "left") {
    int i = v.toInt32();
    style->setLeft(khtml::Length(i, khtml::Fixed));
  } else
    return;

  node.handle()->setStyle(style);
}
