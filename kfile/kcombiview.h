/* This file is part of the KDE libraries
    Copyright (C) 1998 Stephan Kulow <coolo@kde.org>
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

#ifndef _KCOMBIVIEW_H
#define _KCOMBIVIEW_H

#include "kfileinfocontents.h"
#include "knewpanner.h"

class KCombiView: protected KNewPanner, public KFileInfoContents
{
    Q_OBJECT
	
public:
    typedef enum { SimpleView, DetailView, DirList, Custom } FileView;

    KCombiView( FileView dirs, FileView files, 
                bool s, QDir::SortSpec sorting,
		QWidget * parent=0, const char * name=0 );
    ~KCombiView();
    
    virtual QWidget *widget() { return this; }
    virtual void setAutoUpdate(bool);
    virtual void setCurrentItem(const char *filename, const KFileInfo *i);
    virtual void repaint(bool f = true);

    virtual QString findCompletion( const char *base, bool activateFound );

    virtual bool acceptsFiles() { return true; }
    virtual bool acceptsDirs() { return true; }

protected slots:
    
    void dirActivated();
    void fileActivated();
    void fileHighlighted();

protected:
    
    virtual KFileInfoContents *getDirList() { return 0; }
    virtual KFileInfoContents *getFileList() { return 0; }

    virtual void highlightItem(unsigned int item);
    virtual void clearView();
    virtual bool insertItem(const KFileInfo *i, int index);
    
private:
    KFileInfoContents *dirList;
    KFileInfoContents *fileList;
};

#endif
