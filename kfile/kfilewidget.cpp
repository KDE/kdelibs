/* This file is part of the KDE libraries
    Copyright (C) 1999,2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include "kcombiview.h"
#include "kfileiconview.h"
#include "kfiledetailview.h"

#include "kfilewidget.h"

#include "config-kfile.h"


KFileWidget::KFileWidget( FileView view, QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  myDirOperator = new KDirOperator;
  setView( view );
  connectSignals();
  setFocusPolicy( QWidget::WheelFocus );
}


KFileWidget::~KFileWidget()
{
  delete myDirOperator;
}


void KFileWidget::setDirOperator( KDirOperator *dirOp )
{
  ASSERT( dirOp != 0L );

  delete myDirOperator;
  myDirOperator = dirOp;

  connectSignals();
}


void KFileWidget::connectSignals()
{
  connect( myDirOperator, SIGNAL( fileHighlighted( const KFileViewItem * )),
	   SIGNAL( fileHighlighted( const KFileViewItem * )));
  connect( myDirOperator, SIGNAL( fileSelected( const KFileViewItem * )),
	   SIGNAL( fileSelected( const KFileViewItem * )));
  connect( myDirOperator, SIGNAL( dirActivated( const KFileViewItem * )),
	   SIGNAL( dirSelected( const KFileViewItem * )));

  connect( myDirOperator, SIGNAL( urlEntered( const KURL& )),
	   SIGNAL( urlEntered( const KURL& )));

  KFileReader *reader = myDirOperator->fileReader();

  //  connect( reader, SIGNAL( finished() ),
  //	   SLOT( slotFinished() ));
  connect( reader, SIGNAL( error(int, const QString& )),
	   SIGNAL( error(int, const QString&) ));
}

////////////////////////

/*
void KFileWidget::slotFinished()
{
  emit finished();
}
*/


//////////////////////

// now all the forwarders to KDirOperator


void KFileWidget::setURL( const KURL& url )
{
  myDirOperator->setURL( url, true );
}

KURL KFileWidget::url() const
{
    return myDirOperator->url();
}


void KFileWidget::setNameFilter( const QString& nameFilter )
{
  myDirOperator->setNameFilter( nameFilter );
}


void KFileWidget::home()
{
  myDirOperator->home();
}

void KFileWidget::cdUp()
{
  myDirOperator->cdUp();
}

void KFileWidget::back()
{
  myDirOperator->back();
}

void KFileWidget::forward()
{
  myDirOperator->forward();
}

void KFileWidget::rereadDir()
{
  myDirOperator->rereadDir();
}


bool KFileWidget::isRoot() const
{
  return myDirOperator->isRoot();
}


void KFileWidget::setView( FileView view )
{
    KFileView *myFileView;

    switch( view ) {
    case Simple: {
	myFileView = new KFileIconView( this, "simple view" );
	break;
    }
    case Detail: {
	myFileView = new KFileDetailView( this, "detail view" );
	break;
    }
    case SimpleCombi: {
	KCombiView *dirs = new KCombiView( this, "simple combi view");
	dirs->setRight(new KFileIconView( dirs, "simple view" ));
	myFileView = dirs;
	break;
    }
    case DetailCombi: {
	KCombiView *dirs = new KCombiView( this, "detail combi view");
	dirs->setRight(new KFileDetailView( dirs, "detail view" ));
	myFileView = dirs;
	break;
    }
    default: // you have asked for nothing, so don't complain :o)
	myFileView = new KFileIconView( this, "simple view" );
    }

    setFocusProxy( myFileView->widget() );
    myDirOperator->setView( myFileView );
}


void KFileWidget::setView( KFileView *view )
{
  ASSERT( view != 0L );

  setFocusProxy( view->widget() );
  myDirOperator->setView( view );
}


void KFileWidget::setShowHiddenFiles( bool b )
{
    myDirOperator->setShowHiddenFiles( b );
}


bool KFileWidget::showHiddenFiles() const
{
  return myDirOperator->showHiddenFiles();
}


// -------------------


// the forwarders to KFileReader

uint KFileWidget::count() const
{
  return myDirOperator->fileReader()->count();
}


/*
bool KFileWidget::isFinished() const
{
  return myDirOperator->fileReader()->isFinished();
}
*/


bool KFileWidget::isReadable() const
{
  return myDirOperator->fileReader()->isReadable();
}


/////////////////////////////////

// event handling

void KFileWidget::resizeEvent( QResizeEvent * )
{
    myDirOperator->resize( size() );
}

#include "kfilewidget.moc"
