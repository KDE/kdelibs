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
#ifndef RENDER_IMAGE_H
#define RENDER_IMAGE_H

#include "html/dtd.h"
#include "html/html_elementimpl.h"
#include "rendering/render_replaced.h"
#include "dom/dom_string.h"

#include <qmap.h>
#include <qpixmap.h>

namespace khtml {

class DocLoader;
class CachedObject;

class RenderImage : public RenderReplaced
{
public:
    RenderImage(DOM::NodeImpl* _element);
    virtual ~RenderImage();

    virtual const char *renderName() const { return "RenderImage"; }
    virtual void paint( PaintInfo& i, int tx, int ty );

    virtual void layout();

    virtual void setPixmap( const QPixmap &, const QRect&, CachedImage *);

    // don't even think about making these methods virtual!
    QPixmap pixmap() const;
    DOM::HTMLElementImpl* element() const
    { return static_cast<DOM::HTMLElementImpl*>(RenderObject::element()); }

    bool complete() const;

    CachedObject *contentObject() { return image; }
    void setContentObject( CachedObject* );

    // hook to keep RendeObject::m_inline() up to date
    virtual void setStyle(RenderStyle *style);
    virtual void updateFromElement();

    virtual void notifyFinished(CachedObject *finishedObj);
    virtual bool nodeAtPoint(NodeInfo& info, int x, int y, int tx, int ty, HitTestAction hitTestAction, bool inside);

    virtual short calcReplacedWidth() const;
    virtual int   calcReplacedHeight() const;

    virtual SelectionState selectionState() const {return m_selectionState;}
    virtual void setSelectionState(SelectionState s) {m_selectionState = s; }
#if 0
    virtual void caretPos(int offset, int flags, int &_x, int &_y, int &width, int &height);
#endif

private:
    void updateImage(CachedImage* new_image);
    /*
     * Cache for images that need resizing
     */
    QPixmap resizeCache;

    // text to display as long as the image isn't available
    DOM::DOMString alt;

    CachedImage *image;
    CachedImage *oimage;

    bool berrorPic : 1;
    bool loadEventSent : 1;
    SelectionState m_selectionState : 3; // FIXME: don't forget to enlarge this as the enum grows
};


} //namespace

#endif
