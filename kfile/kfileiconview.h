// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow <coolo@kde.org>

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

#ifndef KFILEICONVIEW_H
#define KFILEICONVIEW_H

class KFileViewItem;
class QWidget;
class QLabel;

#include <kiconview.h>
#include <kiconloader.h>
#include <kfileview.h>

#include <kfile.h>

class KFileIconViewItem : public QIconViewItem
{
public:
    KFileIconViewItem( QIconView *parent, const QString &text,
		       const QPixmap &pixmap,
		       const KFileViewItem *fi )
	: QIconViewItem( parent, text, pixmap ), inf( fi ) {}

    virtual ~KFileIconViewItem();

    const KFileViewItem *fileInfo() const {
	return inf;
    }

private:
    const KFileViewItem *inf;

private:
    class KFileIconViewItemPrivate;
    KFileIconViewItemPrivate *d;

};

class KFileIconView : public KIconView, public KFileView
{
    Q_OBJECT

public:
    KFileIconView(QWidget *parent, const char *name);
    virtual ~KFileIconView();

    virtual QWidget *widget() { return this; }
    virtual void clearView();
    virtual void setAutoUpdate( bool ) {}

    virtual void updateView( bool );
    virtual void updateView(const KFileViewItem*);

    virtual void insertItem( KFileViewItem *i );
    virtual void setSelectionMode( KFile::SelectionMode sm );

    virtual void setSelected(const KFileViewItem *, bool);
    virtual bool isSelected(const KFileViewItem *i) const;
    virtual void clearSelection();

    void setIconSize( int size );
    int iconSize() const { return myIconSize; }

    void ensureItemVisible( const KFileViewItem * );

private slots:
    void selected( QIconViewItem *item );
    void highlighted( QIconViewItem *item );
    void showToolTip( QIconViewItem *item );
    void removeToolTip();
    void slotRightButtonPressed( QIconViewItem * );
    void slotDoubleClicked( QIconViewItem * );
    void slotSelectionChanged();

private:
    QLabel *toolTip;
    int th;
    int myIconSize;
    virtual void insertItem(QIconViewItem *, QIconViewItem *) {}
    virtual void setSelectionMode(QIconView::SelectionMode) {}
    virtual void setSelected(QIconViewItem *, bool, bool) {}

private:
    class KFileIconViewPrivate;
    KFileIconViewPrivate *d;
};

#endif // KFILESIMPLEVIEW_H
