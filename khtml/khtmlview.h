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
#include <qstring.h>
#include <qlist.h>

class QPainter;

namespace DOM {
    class HTMLDocumentImpl;
    class HTMLElementImpl;
    class HTMLTitleElementImpl;
    class Range;
};

namespace khtml {
    class RenderObject;
    class RenderRoot;
}

class KHTMLPart;
class KHTMLViewPrivate;

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
     * Constructs a KHTMLView
     */
    KHTMLView( KHTMLPart *part, QWidget *parent, const char *name=0 );
    virtual ~KHTMLView();

    KHTMLPart *part() const { return m_part; }

protected:
    void init();
    void clear();
public:

    int frameWidth() { return _width; }


    /**
     * should the widget follow links automatically, if you click on them?
     * Default is true.
     */
    //    void setFollowsLinks( bool follow );
    /** does the widget follow links automatically?
     */
    //    bool followsLinks();

    /**
     * @return the width of the parsed HTML code. Remember that
     * the documents width depends on the width of the widget.
     */
    //    int docWidth() const;

    /**
     * @return the height of the parsed HTML code. Remember that
     * the documents height depends on the width of the widget.
     */
    //    int docHeight() const;

    /**
     * Causes the widget contents to scroll automatically.  Call
     * @ref #stopAutoScrollY to stop.  Stops automatically when the
     * top or bottom of the document is reached.
     *
     * @param _delay Time in milliseconds to wait before scrolling the
     * document again.
     * @param _dy The amount to scroll the document when _delay elapses.
     *
     * (not implemented)
     */
    //    void autoScrollY( int _delay, int _dy );

    /**
     * Stops the document from @ref #autoScrollY ing.
     */
    //void stopAutoScrollY();

    /**
     * Returns if the widget is currently auto scrolling.
     */
    //    bool isAutoScrollingY()
    //    { return autoScrollYTimer.isActive(); }

    /**
     * Sets the cursor to use when the cursor is on a link.
     */
    void setURLCursor( const QCursor &c )
       { linkCursor = c; }

    /**
     * Returns the cursor which is used when the cursor is on a link.
     */
    const QCursor& urlCursor() { return linkCursor; }

    /**
     * set a margin in x direction
     */
    //    void setMarginWidth(int x) { _marginWidth = x; }
    /**
     * @return the margin With
     */
     int marginWidth() { return _marginWidth; }

    /**
     * set a margin in y direction
     */
    //    void setMarginHeight(int y) { _marginHeight = y; }
    /**
     * @return the margin height
     */
    //    int marginHeight() { return _marginHeight; }

    QString selectedText() const;
    
protected:
    void paintElement( khtml::RenderObject *o, int x, int y );
    void paintSelection();
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void viewportPaintEvent ( QPaintEvent* pe  );
    virtual bool focusNextPrevChild( bool next );

public:
    void layout(bool force = false);
protected:

    virtual void viewportMousePressEvent( QMouseEvent * );

    /**
     * This function emits the 'doubleClick' signal when the user
     * double clicks a &lt;a href=...&gt; tag.
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
protected:
    // ------------------------------------- member variables ------------------------------------

    /*
     * List of all open browsers.
     */
    static QList<KHTMLView> *lstViews;

    /**
     * This is just a temporary variable. It stores the URL the user clicked
     * on, until he releases the mouse again.
     *
     * @ref #mouseMoveHook
     * @ref #mousePressedHook
     */
   QString m_strSelectedURL;

private:

    QCursor linkCursor;

    bool pressed;
    QString overURL;
    int _width;

    int _marginWidth;
    int _marginHeight;

    KHTMLPart *m_part;

    static QPixmap* paintBuffer;

    KHTMLViewPrivate *d;
};

#endif

