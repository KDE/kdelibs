/**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
#ifndef render_replaced_h
#define render_replaced_h

#include "render_box.h"

#include <qwidget.h>
class QScrollView;

namespace khtml {

class RenderReplaced : public RenderBox
{
public:
    RenderReplaced() : RenderBox() {}
    virtual ~RenderReplaced() {}

    virtual const char *renderName() const { return "RenderReplaced"; }

    virtual bool isReplaced() const { return true; }
    virtual bool isRendered() const { return true; }

    virtual void print( QPainter *, int x, int y, int w, int h,
			int tx, int ty);
    virtual void printObject( QPainter *p, int /*x*/, int /*y*/,
			int /*w*/, int /*h*/, int tx, int ty)
    {
	printReplaced(p, tx, ty);
    }


    /**
     * print the replaced object (or position the pixmap)
     */
    virtual void printReplaced(QPainter *p, int _tx, int _ty) = 0;

};


class RenderWidget : public QObject, public RenderReplaced
{
Q_OBJECT
public:
    RenderWidget(QScrollView *view);
    virtual ~RenderWidget();

    virtual void setStyle(RenderStyle *style);

    virtual void printReplaced(QPainter *p, int _tx, int _ty);

    bool isEnabled() const  { return m_widget ? m_widget->isEnabled() : false; }
    void setEnabled(bool enabled) { if(m_widget) m_widget->setEnabled(enabled); }
    virtual short verticalPositionHint() const;

    virtual short intrinsicWidth() const;
    virtual int intrinsicHeight() const;

public slots:
    void slotWidgetDestructed();

protected:
    void setQWidget(QWidget *widget);

protected:
    QScrollView *m_view;
    QWidget *m_widget;
    bool deleted;
};


};

#endif
