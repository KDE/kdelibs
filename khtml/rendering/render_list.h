/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
 */
#ifndef RENDER_LIST_H
#define RENDER_LIST_H

#include "rendering/render_block.h"

// ### list-style-position, list-style-image is still missing

namespace DOM
{
    class DocumentImpl;
}

namespace khtml
{

class RenderListItem;
class RenderListMarker;

class RenderListItem : public RenderBlock
{
    friend class RenderListMarker;

public:
    RenderListItem(DOM::NodeImpl*);

    virtual const char *renderName() const { return "RenderListItem"; }

    virtual void setStyle(RenderStyle *style);

    virtual bool isListItem() const { return true; }

//    long value() const { return m_marker->m_value; }
    void setValue( long v ) { predefVal = v; }
    void calcListValue();
    void calcListTotal();

    virtual void layout( );
    virtual void detach( );
    virtual void calcMinMaxWidth();

    void setInsideList(bool b ) { m_insideList = b; }

protected:

    void updateMarkerLocation();

    RenderListMarker *m_marker;
    signed long predefVal : 30;
    bool m_insideList  : 1;
    bool m_deleteMarker: 1;
};

// -----------------------------------------------------------------------------

class RenderListMarker : public RenderBox
{
public:
    RenderListMarker(DOM::DocumentImpl* document);
    ~RenderListMarker();

    virtual void setStyle(RenderStyle *style);

    virtual const char *renderName() const { return "RenderListMarker"; }
    // so the marker gets to layout itself. Only needed for
    // list-style-position: inside

    virtual void paint(PaintInfo& i, int xoff, int yoff);
    virtual void layout( );
    virtual void calcMinMaxWidth();

    virtual short lineHeight( bool firstLine ) const;
    virtual short baselinePosition( bool firstLine ) const;

    virtual void setPixmap( const QPixmap &, const QRect&, CachedImage *);

    virtual void calcWidth();

    virtual bool isListMarker() const { return true; }

    RenderListItem* listItem() const { return m_listItem; }
    void setListItem(RenderListItem* listItem) { m_listItem = listItem; }

protected:
    friend class RenderListItem;

    bool listPositionInside() const
    { return !m_listItem->m_insideList || style()->listStylePosition() == INSIDE; }

    QString m_item;
    CachedImage *m_listImage;
    int m_value;
    int m_total;
    RenderListItem* m_listItem;
};

} //namespace

#endif
