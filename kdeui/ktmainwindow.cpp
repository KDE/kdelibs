/* This file is part of the KDE libraries
     Copyright
     (C) 2000 Reginald Stadlbauer (reggie@kde.org)
     (C) 1997 Stephan Kulow (coolo@kde.org)
     (C) 1997-2000 Sven Radej (radej@kde.org)
     (C) 1997-2000 Matthias Ettrich (ettrich@kde.org)
     (C) 1999 Chris Schlaeger (cs@kde.org)

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License version 2 as published by the Free Software Foundation.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.

     You should have received a copy of the GNU Library General Public License
     along with this library; see the file COPYING.LIB.  If not, write to
     the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
     Boston, MA 02111-1307, USA.
 */

#include "ktmainwindow.h"

#include <qsessionmanager.h>
#include <qobjectlist.h>

#include <kapp.h>
#include <kconfig.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kmenubar.h>
#include <kstatusbar.h>

#include <klocale.h>
#include <kstddirs.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

KTMainWindow::KTMainWindow( const char *name, WFlags f )
    : KMainWindow( 0, name, f )
{
}

KTMainWindow::~KTMainWindow()
{
}

void KTMainWindow::enableStatusBar( KStatusBar::BarStatus stat )
{
    KStatusBar *sb = 0;
    if ( internalStatusBar() && internalStatusBar()->inherits( "KStatusBar" ) )
	sb = (KStatusBar*)internalStatusBar();
    else
	return;
    if ( ( stat == KStatusBar::Toggle && sb->isVisible() ) || stat == KStatusBar::Hide )
	sb->hide();
    else
	sb->show();
}

void KTMainWindow::enableToolBar( KToolBar::BarStatus stat, int id )
{
    if (idBarMap.find(id) == idBarMap.end())
        return;
    KToolBar *tb = *idBarMap.find( id );
    if ( !tb )
	return;
    bool mystat = tb->isVisible();

    if ( (stat == KToolBar::Toggle && mystat) || stat == KToolBar::Hide )
	tb->hide();
    else
	tb->show();
}

void KTMainWindow::setEnableToolBar( KToolBar::BarStatus stat, const char * name )
{
    KToolBar *tb = KMainWindow::toolBar(name);
    if ( !tb )
	return;
    bool mystat = tb->isVisible();

    if ( (stat == KToolBar::Toggle && mystat) || stat == KToolBar::Hide )
	tb->hide();
    else
	tb->show();
}

int KTMainWindow::addToolBar( KToolBar *toolbar, int index )
{
    int id = idBarMap.count();
    idBarMap.insert( id, toolbar );
    if ( index == -1 )
	QMainWindow::addToolBar( toolbar, (ToolBarDock)toolbar->barPos(), false );
    else
	moveToolBar( toolbar, (ToolBarDock)toolbar->barPos(), false, index );
    return id;
}

bool KTMainWindow::hasMenuBar()
{
    return internalMenuBar() != 0;
}

bool KTMainWindow::hasStatusBar()
{
    return internalStatusBar() != 0;
}

bool KTMainWindow::hasToolBar( int id )
{
    return idBarMap.find( id ) != idBarMap.end();
}

KToolBar *KTMainWindow::toolBar( int id )
{
    if ( idBarMap.find( id ) == idBarMap.end() ) {
        bool honor_mode = (id == 0);
        KToolBar *tb = new KToolBar( this, 0, honor_mode );
        idBarMap.insert( id, tb );
        return tb;
    }
    return *idBarMap.find( id );
}

void KTMainWindow::updateRects()
{
}


#include "ktmainwindow.moc"

