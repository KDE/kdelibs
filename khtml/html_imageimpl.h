/**
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
#ifndef HTML_IMAGEIMPL_H
#define HTML_IMAGEIMPL_H

#include "dtd.h"
#include "html_elementimpl.h"
#include "khtmlio.h"
#include "html_documentimpl.h"

#include "qmap.h"

class QPixmap;

namespace DOM {

class DOMString;

class HTMLImageElementImpl
    : public HTMLPositionedElementImpl, public HTMLImageRequester
{
public:
    HTMLImageElementImpl(DocumentImpl *doc);

    ~HTMLImageElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return IMGStartTag; }
    virtual tagStatus endTag() { return IMGEndTag; }

    virtual bool isFloating() { return hAlign()==Left || hAlign()==Right; }

    bool isMap() const;
    void setIsMap( bool );

    virtual bool isRendered() { return true; }
    virtual void parseAttribute(Attribute *);

    virtual VAlign vAlign() { return valign; }
    virtual int hSpace() {
    	if (isFloating() && hspace.isUndefined())
	    return 2;
	else
	    return hspace.minWidth(width);
    }
    virtual int vSpace() {
    	if (isFloating() && vspace.isUndefined())
	    return 2;
	else
	    return vspace.minWidth(getHeight());
    }

    virtual int getHeight() { return ascent+descent; }

    virtual void calcMinMaxWidth();

    virtual bool mouseEvent( int _x, int _y, int button, MouseEventType type,
				  int _tx, int _ty, DOMString &url);

    virtual void setPixmap( QPixmap * );
    virtual void pixmapChanged( QPixmap * );
    virtual void print(QPainter *p, int _x, int _y, int _w, int _h,
		       int _tx, int _ty);
    virtual void printObject(QPainter *p, int _x, int _y, int _w, int _h,
			     int _tx, int _ty);

    virtual void layout(bool deep = false);
    virtual void attach(KHTMLWidget *w);
    virtual void detach();

protected:
    /*
     * The desired dimensions of the image. If -1, the actual image will
     * determine this value when it is loaded.
     */
    Length predefinedWidth;
    Length predefinedHeight;
    int border;
    Length hspace;
    Length vspace;

    bool ismap;

    int imgHeight;

    /*
     * Pointer to the image
     * If this pointer is 0L, that means that the picture could not be loaded
     * for some strange reason or that the image is waiting to be downloaded
     * from the internet for example.
     */
    QPixmap *pixmap;

    /**
     * The URL of this image.
     */
    DOMString imageURL;

    bool bComplete;
    VAlign valign;

    DOMString usemap;
};


//------------------------------------------------------------------



class HTMLAreaElementImpl : public HTMLElementImpl
{
public:

    enum Shape { Default, Poly, Rect, Circle };

    HTMLAreaElementImpl(DocumentImpl *doc);

    ~HTMLAreaElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual DOMString areaHref() const {
    	return href;
    }

    virtual tagStatus startTag() { return AREAStartTag; }
    virtual tagStatus endTag() { return AREAEndTag; }
    bool noHref() const;

    void setNoHref( bool );
    long tabIndex() const;

    void setTabIndex( long );

    void parseAttribute(Attribute *attr);

    bool isDefault() { return shape==Default; }

    bool mapMouseEvent(int x_, int y_, int width_, int height_,
    	int button_, MouseEventType type_, DOMString& url_);

protected:

    QRegion getRegion(int width_, int height);

    Shape shape;
    QList<Length>* coords;
    DOMStringImpl *href;

    QRegion region;

    int lastw, lasth;
};


// -------------------------------------------------------------------------

class HTMLMapElementImpl : public HTMLElementImpl
{
public:
    HTMLMapElementImpl(DocumentImpl *doc);

    ~HTMLMapElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual DOMString getName() const {
    	return name;
    }

    virtual void parseAttribute(Attribute *attr);

    virtual tagStatus startTag() { return MAPStartTag; }
    virtual tagStatus endTag() { return MAPEndTag; }

    bool mapMouseEvent(int x_, int y_, int width_, int height_,
    	int button_, MouseEventType type_, DOMString& url_);

    static HTMLMapElementImpl* getMap(const DOMString& url_);

private:

    QString name;
    // ### FIxme: this doesn't work, if we have multiple views at the same time.
    // the map has to be somehow attached to the document.
    static QMap<QString,HTMLMapElementImpl*> mapMap;
};


}; //namespace

#endif
