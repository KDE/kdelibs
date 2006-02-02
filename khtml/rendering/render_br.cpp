/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#include "render_br.h"

using namespace khtml;


RenderBR::RenderBR(DOM::NodeImpl* node)
    : RenderText(node, new DOM::DOMStringImpl(QChar('\n')))
{
    m_hasReturn = true;
}

RenderBR::~RenderBR()
{
}

#if 0
void RenderBR::caretPos(int offset, int flags, int &_x, int &_y, int &width, int &height)
{
  RenderText::caretPos(offset,flags,_x,_y,width,height);
  return;
#if 0
    if (previousSibling() && !previousSibling()->isBR() && !previousSibling()->isFloating()) {
        int offset = 0;
        if (previousSibling()->isText())
            offset = static_cast<RenderText*>(previousSibling())->maxOffset();

	// FIXME: this won't return a big width in override mode (LS)
        previousSibling()->caretPos(offset,override,_x,_y,width,height);
        return;
    }

    int absx, absy;
    absolutePosition(absx,absy);
    if (absx == -1) {
        // we don't know out absolute position, and there is no point returning
        // just a relative one
        _x = _y = -1;
    }
    else {
        _x += absx;
        _y += absy;
    }
    height = RenderText::verticalPositionHint( false );
    width = override ? height / 2 : 1;
#endif
}
#endif

FindSelectionResult RenderBR::checkSelectionPoint(int _x, int _y, int _tx, int _ty, DOM::NodeImpl*& node, int &offset, SelPointState &state)
{
  // Simply take result of previous one
  RenderText *prev = static_cast<RenderText *>(previousSibling());
  if (!prev || !prev->isText() || !prev->inlineTextBoxCount() || prev->isBR())
    prev = this;

  //kDebug(6040) << "delegated to " << prev->renderName() << "@" << prev << endl;
  return prev->RenderText::checkSelectionPoint(_x, _y, _tx, _ty, node, offset, state);
}
