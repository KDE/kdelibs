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

#include "kfileinfo.h" 
#include "kcombiview.h"
#include "kdirlistbox.h"
#include "kfiledetaillist.h"
#include "kfilesimpleview.h"
#include "config-kfile.h"

#include <qpainter.h>
#include <qlistbox.h>

#include <qdir.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>

#ifdef Unsorted // the "I hate X.h" modus
#undef Unsorted
#endif

KCombiView::KCombiView(  FileView dirs,  FileView files, 
                         bool s, QDir::SortSpec sorting,
			 QWidget * parent, const char * name ) 
    : QSplitter(Qt::Horizontal, parent, name) , 
      KFileInfoContents(s,sorting)
{
    setSorting( QDir::Unsorted ); // we sort in the part views

    switch (dirs) {
	
    case DirList:
	dirList = new KDirListBox(s, sorting, this, "_dirs");
	break;
    case Custom:
	dirList = getDirList();
	break;
    default:
	fatal("this view for dirs not support");
    }

    switch (files) {
	
    case DetailView:
	fileList = new KFileDetailList(false, sorting, this, "_detail");
	break;
    case SimpleView:
	fileList = new KFileSimpleView(false, sorting, this, "_simple");
	break;
    case DirList: // who ever wants this
	fileList = new KDirListBox(false, sorting, this, "_dirs");
	break;
    case Custom:
	fileList = getFileList();
    }

    /*
    bool showListLabels =  
	kapp->getConfig()->readBoolEntry("ShowListLabels", 
					 DefaultShowListLabels);
    
    if (showListLabels) {
	setLabels(i18n("Folders:"),
		  i18n("Contents:"));
	showLabels(showListLabels);
    }
    */

    KConfig *pConfig = KGlobal::config();
    bool dirsLeft = pConfig->readBoolEntry("ShowDirsLeft", 
						     DefaultShowDirsLeft);

    moveToFirst( dirsLeft ? dirList->widget() : fileList->widget());

    int pan = pConfig->readNumEntry("PannerPosition", 
				    DefaultPannerPosition);
    setRubberband(pan);

    dirList->connectDirSelected(this, SLOT(dirActivated(KFileInfo*)));
    fileList->connectFileSelected(this, SLOT(fileActivated(KFileInfo*)));
    fileList->connectFileHighlighted(this, SLOT(fileHighlighted(KFileInfo*)));
}

KCombiView::~KCombiView() 
{
    KConfig *c = KGlobal::config();
    QString oldgroup = c->group();
    c->setGroup("KFileDialog Settings");
    //    c->writeEntry("PannerPosition", separatorPos(), true, true);
    c->setGroup(oldgroup);
}

void KCombiView::setAutoUpdate(bool f)
{
    fileList->setAutoUpdate(f);
    dirList->setAutoUpdate(f);
}

bool KCombiView::insertItem(const KFileInfo *i, int) 
{
    if (i->isDir()) // for dirs the index is correct
	dirList->addItem(i);
    else
	fileList->addItem(i);

    return true;
}

void KCombiView::clearView()
{
    dirList->clear();
    fileList->clear();
}

void KCombiView::highlightItem(unsigned int)
{
    warning("KCombiView::highlightItem: does nothing");
}

void KCombiView::setCurrentItem(const QString &item, const KFileInfo *i)
{
    if (item != 0) {
	i = 0;
	debugC("setCurrentItem %s",item.local8Bit().data());
	for (uint j = 0; j < count(); j++)
	    if (at(j)->fileName() == item)
		i = at(j);
    }
    
    if (!i) {
	warning("setCurrentItem: no match found.");
	return; 
    }

    if (i->isDir())
	dirList->setCurrentItem(QString::null, i);
    else
	fileList->setCurrentItem(QString::null, i);
    
}

void KCombiView::repaint(bool f)
{
    dirList->repaint(f);
    fileList->repaint(f);
}

QString KCombiView::findCompletion( const char *base, bool )
{
    // first try the files.
    QString found = fileList->findCompletion(base);

    // if no file completion possible, try the dirs
    if (!found) {
	found = dirList->findCompletion(base, true);
    } else {
        // complete in dirList so possible directories are highlighted
        QString foundDir = dirList->findCompletion(base, false);
        // if we find anything -> highlight it and least common denominator
        // is the last result
	if ( !foundDir.isNull() ) {
	    unsigned int i;
            for ( i=1; (i<=found.length()) && (i<=foundDir.length()) ; i++) {
                if ( strncmp( found.ascii(), foundDir.ascii(), i) != 0 )
                    break;
            }
            if ( i==1 ) i++;
	    found.truncate(i-1);
        }
    }
    return found;
}

void KCombiView::dirActivated(KFileInfo *i) 
{
    select(i);
}

void KCombiView::fileActivated(KFileInfo *i)
{
    select(i);
}

void KCombiView::fileHighlighted(KFileInfo *i)
{
    highlight(i);
}

#include "kcombiview.moc"

