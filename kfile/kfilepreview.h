/*  -*- c++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  2000 Werner Trobin <wtrobin@carinthia.com>

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

#ifndef _KFILEPREVIEW_H
#define _KFILEPREVIEW_H

#include <kfileview.h>
#include <kfileviewitem.h>
#include <kfilepreview.h>
#include <kfileiconview.h>
#include <kfiledetailview.h>
#include <kfile.h>

#include <qsplitter.h>
#include <qwidget.h>

#include <qstring.h>
#include <qlabel.h>

#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>

class KFilePreview : public QSplitter, public KFileView
{
    Q_OBJECT

public:
    KFilePreview(QWidget *parent, const char *name);
    KFilePreview(KFileView *view, QWidget *parent, const char *name);
    virtual ~KFilePreview();

    virtual QWidget *widget() { return this; }
    virtual void clearView();

    /**
     * Delets the current view and sets the view to the given @p view.
     * The view is reparented to have this as parent, if necessary.
     */
    void setFileView(KFileView *view);

    /**
     * @returns the current fileview
     */
    KFileView* fileView() const { return left; }

    virtual void updateView( bool );
    virtual void updateView(const KFileViewItem*);
    virtual void removeItem(const KFileViewItem*);

    virtual void setSelectionMode( KFile::SelectionMode sm );

    virtual void clearSelection();
    virtual bool isSelected( const KFileViewItem * ) const;
    virtual void setSelected(const KFileViewItem *, bool);

    virtual void insertSorted(KFileViewItem *tfirst, uint counter);
    virtual void insertItem(KFileViewItem *);
    virtual void clear();

    virtual void setCurrentItem( const KFileViewItem * );
    virtual KFileViewItem * currentFileItem() const;

    virtual void setSorting( QDir::SortSpec sort );
    virtual void sortReversed();

    void ensureItemVisible(const KFileViewItem *);

    void setPreviewWidget(const QWidget *w, const KURL &u);

signals:
    void showPreview(const KURL &);
    void clearPreview();

protected slots:
    void activatedMenu(const KFileViewItem*);
    void selectDir(const KFileViewItem*);
    void highlightFile(const KFileViewItem*);
    void selectFile(const KFileViewItem*);

private:
    void init( KFileView *view );

    bool deleted, previewMode; // ### remove
    KFileView *left;
    QWidget *preview;
    QString viewname;

private:
    class KFilePreviewPrivate;
    KFilePreviewPrivate *d;
};
#endif
