// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
    
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

#ifndef KFILEDETAILLIST_H
#define KFILEDETAILLIST_H

#include <ktablistbox.h>
#include "kfileinfocontents.h"
#include "kfileinfo.h"
#include "kdir.h"

class KFileDetailList : protected KTabListBox, public KFileInfoContents {
    Q_OBJECT
    
public:
    /**
     * Create a file info list widget.
     */
    KFileDetailList(bool s, QDir::SortSpec sorting, QWidget *parent= 0, const char *name= 0);
    virtual ~KFileDetailList();
    
    virtual QWidget *widget() { return this; }
    virtual void setAutoUpdate(bool);
    virtual void clearView();
    virtual void repaint(bool f = true);
    
    virtual bool acceptsFiles() { return true; }
    virtual bool acceptsDirs() { return true; }

protected:
    virtual void highlightItem(unsigned int item);
    virtual bool insertItem(const KFileInfo *i, int index);
    virtual void keyPressEvent( QKeyEvent *e);
    virtual void focusInEvent ( QFocusEvent *e );

protected slots:
    void reorderFiles(int inColumn);
    void selected(int);
    void highlighted(int);
};

#endif // KFILEDETAILLIST_H
