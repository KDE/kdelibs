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

// $Id$

#include "kfilepreview.h"
#include "kdirlistbox.h"
#include "kfiledetaillist.h"
#include "kfileinfo.h"
#include "kfilesimpleview.h"
#include "kpreview.h"
#include "kfilepreview.h"

#include <qpainter.h>
#include <qlistbox.h>

#include <qdir.h>

#include <kapp.h>

#ifdef Unsorted // the "I hate X.h" modus
#undef Unsorted
#endif

KFilePreview::KFilePreview(   
                         const KDir *inDir, bool s, QDir::SortSpec sorting, 
			 QWidget * parent, const char * name ) 
    : KNewPanner(parent, name, KNewPanner::Vertical) , 
      KFileInfoContents(s,sorting)
{

    fileList = new KFileSimpleView(s, sorting, this, "_dirs");

    myPreview = new KPreview( inDir, this, "_preview" );

    activate(fileList->widget(), myPreview);

    int pan = kapp->getConfig()->readNumEntry("PannerPosition", 50);
    setSeparatorPos(pan);

    fileList->connectDirSelected(this, SLOT(dirActivated()));
    fileList->connectFileSelected(this, SLOT(fileActivated()));
    fileList->connectFileHighlighted(this, SLOT(fileHighlighted()));
}

void KFilePreview::setAutoUpdate(bool f)
{
    fileList->setAutoUpdate(f);
}

bool KFilePreview::insertItem(const KFileInfo *i, int) 
{
    fileList->addItem(i);
    return true;
}

void KFilePreview::clearView()
{
    fileList->clear();
}

void KFilePreview::highlightItem(unsigned int)
{
    warning("KCombiView::highlightItem: does nothing");
}

void KFilePreview::setCurrentItem(const char *item, const KFileInfo *i)
{
    if (item != 0) {
	i = 0;
	debugC("setCurrentItem %s",item);
	for (uint j = 0; j < count(); j++)
	    if (at(j)->fileName() == item)
		i = at(j);
    }
    
    if (!i) {
	warning("setCurrentItem: no match found.");
	return; 
    }

    fileList->setCurrentItem(0, i);
    
}

void KFilePreview::repaint(bool f)
{
    fileList->repaint(f);
}

QString KFilePreview::findCompletion( const char *base, bool )
{
    // try files and directories
    QString found = fileList->findCompletion(base);

    return found;
}

void KFilePreview::dirActivated() 
{
    select(fileList->selectedDir());
}

void KFilePreview::fileActivated()
{
    select(fileList->selectedFile());
}

void KFilePreview::fileHighlighted()
{
    highlight(fileList->highlightedFile());
    myPreview->previewFile(fileList->highlightedFile());
}

#include "kfilepreview.moc"

