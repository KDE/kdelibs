/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

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
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Frames
// $Id$

#ifndef HTMLFRAME_H
#define HTMLFRAME_H

#include <qwidget.h>
#include <qlist.h>
#include <qframe.h>

class HTMLFramePanner;
class HTMLFrameSet;
class KHTMLView;

class HTMLFramePanner : public QFrame
{
    Q_OBJECT
public:
    enum Orientation { HORIZONTAL = 1, VERTICAL = 2 };
    
    HTMLFramePanner( HTMLFramePanner::Orientation _orientation, QWidget *_parent = 0L, const char *_name = 0L );
    ~HTMLFramePanner();
    
    void setChild1( QWidget *_child ) { child1 = _child; }
    void setChild2( QWidget *_child ) { child2 = _child; }    

    void setIsMoveable( bool _move );

protected:
    virtual void mousePressEvent( QMouseEvent *_ev );
    virtual void mouseMoveEvent( QMouseEvent *_ev );
    virtual void mouseReleaseEvent( QMouseEvent *_ev );
    
    QWidget *child1;
    QWidget *child2;

    int initialX;
    int initialY;
    QPoint initialGlobal;
    
    HTMLFramePanner::Orientation orientation;

    bool moveable;
};

/**
 * HTML Frame Set
 * @internal
 */
class HTMLFrameSet : public QWidget
{
    Q_OBJECT

    // FIXME Implement the missing functions (Lars)
    friend class KHTMLWidget;
public:
    HTMLFrameSet( QWidget *_parent,
                  QString _cols, QString _rows,
                  int _frameBorder, bool _bAllowResize);
    ~HTMLFrameSet();
    
    void append( QWidget *_w );

    virtual void parse();
  
    virtual int calcSize( QString s, int _max );

    KHTMLView* getSelectedFrame();

    /**
     * @return TRUE if the user is allowed to resize the frame set.
     *
     * @see #bAllowResize
     */
    bool getAllowResize() { return bAllowResize; }
    /**
     * @return the width of the frames border ( read: @ref HTMLFramePanner ) in pixels
     *         or -1 for the default width.
     */
    int getFrameBorder() { return frameBorder; }
  
protected:
    virtual void resizeEvent( QResizeEvent* _ev );
    
    QList<QWidget> widgetList;

    HTMLFramePanner::Orientation orientation;

    QString cols;
    QString rows;
    
    /**
     * Array that holds the layout information for all embedded frames.
     */
    QArray<int> size;
    /**
     * Amount of frames as mentioned in the COLS or ROWS tag.
     */
    int elements;

    /**
     * The amount of frames we parsed until now.
     */
    int cFrames;

    /**
     * The amount of pixels used for the frames border ( read @ref HTMLFramePanner ).
     * A value of -1 indicates the default.
     */
    int frameBorder;
  
    /**
     * This flags is usually TRUE. But if we have the &lt;frame noresize&gt; tag, 
     * this flag is set to FALSE to indicate that the user may not resize this frame set.
     */
    bool bAllowResize;

    /**
     * This is the last panner added to the widgetList. This variable is used
     * during parsing only.
     */
    HTMLFramePanner *lastPanner;
};



#endif



















