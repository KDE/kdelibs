/*
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
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

#ifndef HTML_BASEIMPL_H
#define HTML_BASEIMPL_H 1

#include "dtd.h"
#include "html_elementimpl.h"
#include "khtmlio.h"
#include "qscrollview.h"

class KHTMLWidget;

namespace DOM {

class DOMString;

class HTMLBodyElementImpl : public HTMLBlockElementImpl,
    public HTMLImageRequester
{
public:
    HTMLBodyElementImpl(DocumentImpl *doc, KHTMLWidget *view = 0);

    ~HTMLBodyElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return BODYStartTag; }
    virtual tagStatus endTag() { return BODYEndTag; }

    virtual void parseAttribute(Attribute *);
    virtual void setStyle(CSSStyle *currentStyle);

    virtual void close();

    virtual void attach(KHTMLWidget *);
    virtual void detach();
    virtual void setPixmap( QPixmap * );
    virtual void pixmapChanged( QPixmap * );
    
    virtual void print( QPainter *, int x, int y, int w, int h,
			int tx, int ty);

protected:
    KHTMLWidget *view;
    DOMString bgURL;
    QPixmap* bgPixmap;
};

// -------------------------------------------------------------------------

class HTMLFrameElementImpl : public HTMLPositionedElementImpl
{
    friend class KHTMLWidget;
public:
    HTMLFrameElementImpl(DocumentImpl *doc);

    ~HTMLFrameElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return FRAMEStartTag; }
    virtual tagStatus endTag() { return FRAMEEndTag; }

    virtual void parseAttribute(Attribute *);
    virtual void layout(bool);

    virtual bool isInline() { return false; }
    virtual void attach(KHTMLWidget *w);
    virtual void detach();

    bool noResize() { return noresize; }

protected:
    DOMString url;
    DOMString name;
    KHTMLWidget *view;
    KHTMLWidget *parentWidget;

    bool frameBorder;
    int marginWidth;
    int marginHeight;
    bool noresize;
    QScrollView::ScrollBarMode scrolling;
};

// -------------------------------------------------------------------------

class HTMLFrameSetElementImpl : public HTMLPositionedElementImpl
{
    friend class KHTMLWidget;
public:
    HTMLFrameSetElementImpl(DocumentImpl *doc);

    ~HTMLFrameSetElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return FRAMESETStartTag; }
    virtual tagStatus endTag() { return FRAMESETEndTag; }

    virtual void parseAttribute(Attribute *);
    virtual void layout(bool);

    virtual bool isInline() { return false; }
    virtual NodeImpl *addChild(NodeImpl *child);
    void close();
    virtual void attach(KHTMLWidget *w);

    virtual bool mouseEvent( int _x, int _y, int button, MouseEventType type,
		     int _tx, int _ty, DOMString &url);

    bool frameBorder() { return frameborder; }
    bool noResize() { return noresize; }

protected:
    void positionFrames(bool);


    QList<Length> *rows;
    QList<Length> *cols;
    int *rowHeight;
    int *colWidth;
    int totalRows;
    int totalCols;

    // mozilla and other's use this in the frameset, although it's not standard html4
    bool frameborder;
    int border;
    bool noresize;

    KHTMLWidget *view;

    bool resizing;  // is the user resizing currently
    int hSplit;     // the split currently resized
    int vSplit;
    int hSplitPos;
    int vSplitPos;
    bool *hSplitVar; // is this split variable?
    bool *vSplitVar;
};

// -------------------------------------------------------------------------

class HTMLHeadElementImpl : public HTMLElementImpl
{
public:
    HTMLHeadElementImpl(DocumentImpl *doc);

    ~HTMLHeadElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual bool isInline() { return false; }

    virtual tagStatus startTag() { return HEADStartTag; }
    virtual tagStatus endTag() { return HEADEndTag; }

    virtual void print(QPainter *, int, int, int, int, int, int) {}
    virtual void layout(bool) {}

};

// -------------------------------------------------------------------------

class HTMLHtmlElementImpl : public HTMLPositionedElementImpl
{
public:
    HTMLHtmlElementImpl(DocumentImpl *doc);

    ~HTMLHtmlElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return HTMLStartTag; }
    virtual tagStatus endTag() { return HTMLEndTag; }

    virtual int getWidth() const { return width; }
    virtual NodeImpl *addChild(NodeImpl *child);

    virtual void getAbsolutePosition(int &xPos, int &yPos);
    virtual bool isInline() { return false; }

    virtual void layout(bool);

    virtual void attach(KHTMLWidget *);

protected:
    KHTMLWidget *view;
};

}; //namespace

#endif

