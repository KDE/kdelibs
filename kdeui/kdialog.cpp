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

#include <qabstractlayout.h>
#include <qobjectlist.h>
#include <qguardedptr.h>
#include <qlineedit.h>
#include <qvaluelist.h>
#include <qtimer.h>
#include <netwm.h>

#include <kapplication.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

int KDialog::mMarginSize = 11;
int KDialog::mSpacingSize = 6;

template class QPtrList<QLayoutItem>;

KDialog::KDialog(QWidget *parent, const char *name, bool modal, WFlags f)
  : QDialog(parent, name, modal, f)
{
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
      case Key_Escape:
      case Key_Enter:
      case Key_Return:
      {
        if(testWFlags(WType_Dialog | WShowModal))
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
      if ( e->state() == ControlButton && 
           qApp->focusWidget()  && 
           qApp->focusWidget()->inherits( "QTextEdit" ) &&
           (e->key() == Key_Return || e->key() == Key_Enter) )
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
  return( mMarginSize );
}


int KDialog::spacingHint()
{
  return( mSpacingSize );
}


void KDialog::polish()
{
  QDialog::polish();
  QWidget *w = focusWidget();
  if( w && w->isEnabled() && w->inherits( "QLineEdit" ) &&
      !w->inherits( "KPasswordEdit" ))
    ((QLineEdit*)w)->selectAll();
}


void KDialog::setCaption( const QString &_caption )
{
  QString caption = kapp ? kapp->makeStdCaption( _caption ) : _caption;
  setPlainCaption( caption );
}


void KDialog::setPlainCaption( const QString &caption )
{
  QDialog::setCaption( caption );

#ifdef Q_WS_X11 //FIXME(E) Implement for Qt/E
  NETWinInfo info( qt_xdisplay(), winId(), qt_xrootwin(), 0 );
  info.setName( caption.utf8().data() );
#endif
}


void KDialog::resizeLayout( QWidget *w, int margin, int spacing )
{
  if( w->layout() )
  {
    resizeLayout( w->layout(), margin, spacing );
  }

  if( w->children() )
  {
    QObjectList *l = (QObjectList*)w->children(); // silence please
    for( uint i=0; i < l->count(); i++ )
    {
      QObject *o = l->at(i);
      if( o->isWidgetType() )
      {
	resizeLayout( (QWidget*)o, margin, spacing );
      }
    }
  }
}


void KDialog::resizeLayout( QLayoutItem *lay, int margin, int spacing )
{
  QLayoutIterator it = lay->iterator();
  QLayoutItem *child;
  while ( (child = it.current() ) )
  {
    resizeLayout( child, margin, spacing );
    ++it;
  }
  if( lay->layout() != 0 )
  {
    lay->layout()->setMargin( margin );
    lay->layout()->setSpacing( spacing );
  }
}

void KDialog::centerOnScreen( QWidget *w, int screen )
{
  if ( !w )
    return;

  QDesktopWidget *desktop = QApplication::desktop();
  if ( screen < 0 || screen < desktop->numScreens() )
  {
    screen = desktop->screenNumber( w );
    if ( screen == -1 )
      screen = desktop->primaryScreen();
  }

  QRect r = desktop->screenGeometry( screen );
  w->move( r.center().x() - w->width()/2,
           r.center().y() - w->height()/2 );
}

class KDialogQueuePrivate
{
public:
  QValueList< QGuardedPtr<QDialog> > queue;
  bool busy;
};

static KStaticDeleter<KDialogQueue> ksdkdq;

KDialogQueue *KDialogQueue::_self=0;

KDialogQueue* KDialogQueue::self()
{
   if (!_self)
      _self = ksdkdq.setObject(new KDialogQueue);
   return _self;
}

KDialogQueue::KDialogQueue()
{
   d = new KDialogQueuePrivate;
   d->busy = false;
}

KDialogQueue::~KDialogQueue()
{
   delete d; d = 0;
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
      if(!d->queue.count())
         return;
      dialog = d->queue.first();
      d->queue.remove(d->queue.begin());
   }
   while(!dialog);

   d->busy = true;
   dialog->exec();
   d->busy = false;
   delete dialog;

   if (d->queue.count())
      QTimer::singleShot(20, this, SLOT(slotShowQueuedDialog()));
   else
      ksdkdq.destructObject(); // Suicide.
}

void KDialog::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdialog.moc"
