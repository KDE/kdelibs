/*
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
#ifndef RENDER_BR_H
#define RENDER_BR_H

#include "render_text.h"

/*
 * The whole class here is a hack to get <br> working, as long as we don't have support for
 * CSS2 :before and :after pseudo elements
 */
namespace khtml {

class RenderBR : public RenderText
{
public:
    RenderBR(DOM::NodeImpl* node);
    virtual ~RenderBR();

    virtual const char *renderName() const { return "RenderBR"; }

    virtual void paint( PaintInfo&, int, int) {}

    virtual unsigned int width(unsigned int, unsigned int, const Font *) const { return 0; }
    virtual unsigned int width( unsigned int, unsigned int, bool) const { return 0; }
    virtual short width() const { return RenderText::width(); }

    virtual int height() const { return 0; }

    // overrides
    virtual void calcMinMaxWidth() {}
    virtual short minWidth() const { return 0; }
    virtual short maxWidth() const { return 0; }

    virtual FindSelectionResult checkSelectionPoint( int _x, int _y, int _tx, int _ty,
                                                     DOM::NodeImpl*& node, int & offset,
						     SelPointState & );

    virtual bool isBR() const { return true; }
    virtual void caretPos(int offset, bool override, int &_x, int &_y, int &width, int &height);
    /** returns the lowest possible value the caret offset may have to
     * still point to a valid position.
     *
     * Returns 0.
     */
    virtual long minOffset() const { return 0; }
    /** returns the highest possible value the caret offset may have to
     * still point to a valid position.
     *
     * Returns also 0, as BRs have no width.
     */
    virtual long maxOffset() const { return 0; }
};

}
#endif
