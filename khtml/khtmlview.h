/* This file is part of the KDE project

   Copyright (C) 1997 Martin Jones (mjones@kde.org)
             (C) 1998 Waldo Bastian (bastian@kde.org)
             (C) 1998, 1999 Torben Weis (weis@kde.org)
             (C) 1999 Lars Knoll (knoll@kde.org)
	     (C) 1999 Antti Koivisto (koivisto@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KHTML_H
#define KHTML_H

// qt includes and classes
#include <qscrollview.h>

template<class C> class QList;

class QPainter;

namespace DOM {
    class HTMLDocumentImpl;
    class HTMLElementImpl;
    class HTMLTitleElementImpl;
    class Range;
    class NodeImpl;
};

namespace khtml {
    class RenderObject;
    class RenderRoot;
}

class KHTMLPart;
class KHTMLViewPrivate;

/**
 * Render and display HTML in a @ref QScrollView.
 *
 * Suitable for use as an application's main view.
 **/
class KHTMLView : public QScrollView
{
    Q_OBJECT

    friend DOM::HTMLDocumentImpl;
    friend DOM::HTMLElementImpl;
    friend DOM::HTMLTitleElementImpl;
    friend class KHTMLPart;
    friend khtml::RenderRoot;

public:
    /**
     * Construct a @ref KHTMLView.
     */
    KHTMLView( KHTMLPart *part, QWidget *parent, const char *name=0 );
    virtual ~KHTMLView();

    /**
     * Retrieve a pointer to the @ref KHTMLPart that is
     * rendering the page.
     **/
    KHTMLPart *part() const { return m_part; }

    int frameWidth() const { return _width; }

    /**
     * Implements keyboard traversal.
     *
     * Sets the view's position so that the actual link
     * is visible.
     **/
    bool gotoNextLink();
    /**
     * Implements keyboard traversal.
     *
     * Sets the view's position so that the actual link
     * is visible.
     **/
    bool gotoPrevLink();
    void toggleActLink(bool);

    /**
     * Set a margin in x direction.
     */
    void setMarginWidth(int x) { _marginWidth = x; }

    /**
     * Retrieve the margin width.
     *
     * A return value of -1 means the default value will be used.
     */
    int marginWidth() const { return _marginWidth; }

    /*
     * Set a margin in y direction.
     */
    void setMarginHeight(int y) { _marginHeight = y; }

    /*
     * Retrieve the margin height.
     *
     * A return value of -1 means the default value will be used.
     */
    int marginHeight() { return _marginHeight; }

    /**
     * Print the HTML document.
     **/
    void print();

    void layout(bool force = false);

    static const QList<KHTMLView> *viewList() { return lstViews; }

protected:
    void clear();

    void paintElement( khtml::RenderObject *o, int x, int y );
    virtual void resizeEvent ( QResizeEvent * event );
    virtual bool focusNextPrevChild( bool next );
    virtual void drawContents ( QPainter * p, int clipx, int clipy, int clipw, int cliph );

    virtual void viewportMousePressEvent( QMouseEvent * );

    /**
     * This function emits the @ref doubleClick() signal when the user
     * double clicks a <a href=...> tag.
     */
    virtual void viewportMouseDoubleClickEvent( QMouseEvent * );

    /**
     * This function is called when the user moves the mouse.
     */
    virtual void viewportMouseMoveEvent(QMouseEvent *);

    /**
     * this function is called when the user releases a mouse button.
     */
    virtual void viewportMouseReleaseEvent(QMouseEvent *);

    void keyPressEvent( QKeyEvent *_ke );
    void keyReleaseEvent( QKeyEvent *_ke );

    /**
     * Scroll the view
     */
    void doAutoScroll();

public slots:
    /**
     * @internal
     */
    void triggerResize();

private:
    void init();

    bool gotoLink();

    void followLink();

    // ------------------------------------- member variables ------------------------------------

    /**
     * List of all open browsers.
     */
    static QList<KHTMLView> *lstViews;

    DOM::NodeImpl *nodeUnderMouse() const;

    int _width;
    int _height;

    int _marginWidth;
    int _marginHeight;

    KHTMLPart *m_part;

    static QPixmap* paintBuffer;

    KHTMLViewPrivate *d;
};

#endif

