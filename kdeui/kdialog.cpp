/*  This file is part of the KDE Libraries
 *  Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
 *  Additions 1999-2000 by Espen Sand (espen@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <kconfig.h>
#include <kapplication.h>
#include <kdialog.h>
#include <kwhatsthismanager_p.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#include <qlayout.h>
#include <qobject.h>
#include <qpointer.h>
#include <qlineedit.h>
#include <q3valuelist.h>
#include <qtimer.h>
#include <qcursor.h>
#include <qx11info_x11.h>
#include <QKeyEvent>
#include <QDesktopWidget>

#include "config.h"
#ifdef Q_WS_X11
#include <netwm.h> 
#endif

const int KDialog::mMarginSize = 11;
const int KDialog::mSpacingSize = 6;

template class Q3PtrList<QLayoutItem>;

KDialog::KDialog(QWidget *parent, const char *name, bool modal, Qt::WFlags f)
  : QDialog(parent, name, modal, f), d(0)
{
    KWhatsThisManager::init ();
}

//
// Grab QDialogs keypresses if non-modal.
//
void KDialog::keyPressEvent(QKeyEvent *e)
{
  if ( e->state() == 0 )
  {
    switch ( e->key() )
    {
      case Qt::Key_Escape:
      case Qt::Key_Enter:
      case Qt::Key_Return:
      {
        if(testAttribute(Qt::WA_ShowModal) && ( windowFlags() & Qt::Dialog ) )
	{
          QDialog::keyPressEvent(e);
	}
        else
        {
   	  e->ignore();
        }
      }
      break;
      default:
	e->ignore();
	return;
    }
  }
  else
  {
      // accept the dialog when Ctrl-Return is pressed
      if ( e->state() == Qt::ControlModifier &&
           (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) )
      {
          e->accept();
          accept();
      }
      else
      {
          e->ignore();
      }
  }
}


int KDialog::marginHint()
{
  return mMarginSize;
}


int KDialog::spacingHint()
{
  return mSpacingSize;
}


void KDialog::setCaption( const QString &_caption )
{
  QString caption = kapp ? kapp->makeStdCaption( _caption ) : _caption;
  setPlainCaption( caption );
}


void KDialog::setPlainCaption( const QString &caption )
{
  QDialog::setCaption( caption );

#ifdef Q_WS_X11
  NETWinInfo info( QX11Info::display(), winId(), QX11Info::appRootWindow(), 0 );
  info.setName( caption.toUtf8().data() );
#endif
}


void KDialog::resizeLayout( QWidget *w, int margin, int spacing )
{
  if( w->layout() )
  {
    resizeLayout( w->layout(), margin, spacing );
  }

  if ( w->children().count() > 0 )
  {
	  QList<QObject*> l = w->children();
	  foreach ( QObject *o, l ) {
		  if( o->isWidgetType() )
		  {
			  resizeLayout( (QWidget*)o, margin, spacing );
		  }
	  }
  }
}


void KDialog::resizeLayout( QLayout *lay, int margin, int spacing )
{
  QLayoutItem *child;
  int pos = 0;
  while ( (child = lay->itemAt(pos) ) )
  {
    if ( child->layout() )
      resizeLayout( child->layout(), margin, spacing );
    ++pos;
  }
  if( lay->layout() )
  {
    lay->layout()->setMargin( margin );
    lay->layout()->setSpacing( spacing );
  }
}

static QRect screenRect( QWidget *w, int screen )
{
  QDesktopWidget *desktop = QApplication::desktop();
  KConfig gc("kdeglobals", false, false);
  gc.setGroup("Windows");
  if (desktop->isVirtualDesktop() &&
      gc.readBoolEntry("XineramaEnabled", true) &&
      gc.readBoolEntry("XineramaPlacementEnabled", true)) {
    if ( screen < 0 || screen >= desktop->numScreens() ) {
      if ( screen == -1 ) {
        screen = desktop->primaryScreen();
      } else if ( screen == -3 ) {
        screen = desktop->screenNumber( QCursor::pos() );
      } else {
        screen = desktop->screenNumber( w );
      }
    }
    return desktop->screenGeometry(screen);
  } else {
    return desktop->geometry();
  }
}

void KDialog::centerOnScreen( QWidget *w, int screen )
{
  if ( !w )
    return;
  QRect r = screenRect( w, screen );

  w->move( r.center().x() - w->width()/2,
           r.center().y() - w->height()/2 );
}

bool KDialog::avoidArea( QWidget *w, const QRect& area, int screen )
{
  if ( !w )
    return false;
  QRect fg = w->frameGeometry();
  if ( !fg.intersects( area ) )
    return true; // nothing to do.

  QRect scr = screenRect( w, screen );
  QRect avoid( area ); // let's add some margin
  avoid.moveBy( -5, -5 );
  avoid.rRight() += 10;
  avoid.rBottom() += 10;

  if ( qMax( fg.top(), avoid.top() ) <= qMin( fg.bottom(), avoid.bottom() ) )
  {
    // We need to move the widget up or down
    int spaceAbove = qMax(0, avoid.top() - scr.top());
    int spaceBelow = qMax(0, scr.bottom() - avoid.bottom());
    if ( spaceAbove > spaceBelow ) // where's the biggest side?
      if ( fg.height() <= spaceAbove ) // big enough?
        fg.setY( avoid.top() - fg.height() );
      else
        return false;
    else
      if ( fg.height() <= spaceBelow ) // big enough?
        fg.setY( avoid.bottom() );
      else
        return false;
  }

  if ( qMax( fg.left(), avoid.left() ) <= qMin( fg.right(), avoid.right() ) )
  {
    // We need to move the widget left or right
    int spaceLeft = qMax(0, avoid.left() - scr.left());
    int spaceRight = qMax(0, scr.right() - avoid.right());
    if ( spaceLeft > spaceRight ) // where's the biggest side?
      if ( fg.width() <= spaceLeft ) // big enough?
        fg.setX( avoid.left() - fg.width() );
      else
        return false;
    else
      if ( fg.width() <= spaceRight ) // big enough?
        fg.setX( avoid.right() );
      else
        return false;
  }
  //kdDebug() << "Moving window to " << fg.x() << "," << fg.y() << endl;
  w->move(fg.x(), fg.y());
  return true;
}

class KDialogQueuePrivate
{
public:
  QList< QPointer<QDialog> > queue;
  bool busy;
};

static KStaticDeleter<KDialogQueue> ksdkdq;

KDialogQueue *KDialogQueue::_self=0;

KDialogQueue* KDialogQueue::self()
{
   if (!_self)
      _self = ksdkdq.setObject(_self, new KDialogQueue);
   return _self;
}

KDialogQueue::KDialogQueue() : d(new KDialogQueuePrivate)
{
   d->busy = false;
}

KDialogQueue::~KDialogQueue()
{
   delete d;
   _self = 0;
}

// static
void KDialogQueue::queueDialog(QDialog *dialog)
{
   KDialogQueue *_this = self();
   _this->d->queue.append(dialog);
   QTimer::singleShot(0, _this, SLOT(slotShowQueuedDialog()));
}

void KDialogQueue::slotShowQueuedDialog()
{
   if (d->busy)
      return;
   QDialog *dialog;
   do {
       if(d->queue.isEmpty())
         return;
      dialog = d->queue.first();
      d->queue.pop_front();
   }
   while(!dialog);

   d->busy = true;
   dialog->exec();
   d->busy = false;
   delete dialog;

   if (!d->queue.isEmpty())
      QTimer::singleShot(20, this, SLOT(slotShowQueuedDialog()));
   else
      ksdkdq.destructObject(); // Suicide.
}

void KDialog::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdialog.moc"
