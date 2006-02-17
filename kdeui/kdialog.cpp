/*  This file is part of the KDE Libraries
 *  Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
 *  Additions 1999-2000 by Espen Sand (espen@kde.org)
 *                      by Holger Freyther <freyther@kde.org>
 *            2005-2006 by Olivier Goffart (ogoffart at kde.org)
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kdialog.h"

#include <qlayout.h>
#include <qpointer.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <qcursor.h>
#include <QDesktopWidget>
#include <qlayout.h>
#include <QKeyEvent>
#include <QHideEvent>
#include <QDesktopWidget>
#include <QWhatsThis>
#include <QApplication>
#include <QHash>
#include <QSignalMapper>

#include <ktoolinvocation.h>
#include <klocale.h>
#include <kconfig.h>
#include <kpushbutton.h>
#include <kapplication.h>
#include <kwhatsthismanager_p.h>
#include <kdebug.h>
#include <kstaticdeleter.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kseparator.h>
#include <kurllabel.h>

#include "config.h"
#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <netwm.h>
#endif

#include <stdlib.h>

const int KDialog::mMarginSize = 11;
const int KDialog::mSpacingSize = 6;

struct KDialog::Private {
	Private() : bDetails(false), bSettingDetails(false), detailsWidget(0), 
		mTopLayout(0), mMainWidget(0), mUrlHelp(0), mActionSep(0),
		 mButtonOrientation(Qt::Horizontal) , 
		buttonBox(0) , buttonMask(0) , buttonStyle(0)  { }
	
	void resizeButton( bool sameWidth, int margin, int spacing, int orientation );

    bool bDetails;
    bool bSettingDetails;
    QWidget *detailsWidget;
    QSize incSize;
    QSize minSize;
    QString detailsButton;

	QBoxLayout   *mTopLayout;
    QWidget      *mMainWidget;
    KUrlLabel    *mUrlHelp;
    KSeparator   *mActionSep;

    QString mAnchor;
    QString mHelpApp;
    QString mHelpLinkText;

	Qt::Orientation  mButtonOrientation;
    ButtonCode mEscapeButton;

	QWidget *buttonBox;
	int buttonMask;
	int buttonStyle;
	QHash<int,KPushButton*> buttonList;
	QSignalMapper buttonSignalMap;

};


#ifdef KDE3_SUPPORT
KDialog::KDialog(QWidget *parent, const char *name, bool modal, Qt::WFlags f)
  : QDialog(parent, name, modal, f), d(new Private)
{
	KWhatsThisManager::init ();
	connect( this, SIGNAL(layoutHintChanged()), SLOT(updateGeometry()) );
	connect( & d->buttonSignalMap , SIGNAL(mapped(int)) , this , SLOT(slotButtonClicked(int) ) );
	setButtonMask( 0 );
}
#endif


KDialog::KDialog( QWidget *parent, const QString &caption  ,
		ButtonCodes buttonMask ,  Qt::WFlags f  ,
		const KGuiItem &user1,  const KGuiItem &user2, const KGuiItem &user3 )
    :  QDialog( parent, f |  Qt::WStyle_DialogBorder),  d(new Private)
{
  setCaption( caption );
  setModal( true );
  KWhatsThisManager::init();

  connect( this, SIGNAL(layoutHintChanged()), SLOT(updateGeometry()) );
  connect( & d->buttonSignalMap , SIGNAL(mapped(int)) , this , SLOT(slotButtonClicked(int) ) );

  setButtonMask( buttonMask, user1, user2, user3 );
}


KDialog::~KDialog()
{
  delete d;
}


void KDialog::setButtonMask( ButtonCodes buttonMask ,
				 const KGuiItem &user1, const KGuiItem &user2,
				 const KGuiItem &user3 )
{
	if(d->buttonBox)
	{
		d->buttonList.clear();
		delete d->buttonBox;
	}
	
	if( buttonMask & Cancel ) { buttonMask &= ~Close; }
	if( buttonMask & Apply ) { buttonMask &= ~Try; }
	if( buttonMask & Details ) { buttonMask &= ~Default; }
	
	d->buttonMask = buttonMask;

	if( buttonMask == 0 )
	{
		return; // When we want no button box
		setupLayout();
	}

	d->mEscapeButton = (buttonMask&Cancel) ? Cancel : Close;
	d->buttonBox = new QWidget( this );

	if( d->buttonMask & Help )
		appendButton( Help, KStdGuiItem::help() );
	if( d->buttonMask & Default )
		appendButton( Default, KStdGuiItem::defaults() );
	if( d->buttonMask & User3 )
		appendButton( User3, user3 );
	if( d->buttonMask & User2 )
		appendButton( User2, user2 );
	if( d->buttonMask & User1 )
		appendButton( User1, user1 );
	if( d->buttonMask & Ok )
		appendButton( Ok, KStdGuiItem::ok() );
	if( d->buttonMask & Apply )
		appendButton( Apply, KStdGuiItem::apply() );
	if( d->buttonMask & Try )
		appendButton( Try, i18n( "&Try" ) );
	if( d->buttonMask & Cancel )
		appendButton( Cancel, KStdGuiItem::cancel() );
	if( d->buttonMask & Close )
		appendButton( Close, KStdGuiItem::close() );
	if( d->buttonMask & Yes )
		appendButton( Yes, KStdGuiItem::yes() );
	if( d->buttonMask & No )
		appendButton( No, KStdGuiItem::no() );
	if( d->buttonMask & Details )
	{
		appendButton( Details, QString() );
		setDetails(false);
	}

	setButtonStyle( KGlobalSettings::buttonLayout() );
}



void KDialog::setButtonStyle( int style )
{
	if( !d->buttonBox )
		return;

	if( style < 0 || style > ActionStyleMAX ) { style = ActionStyle0; }
	d->buttonStyle = style;

	const int *layout;
	if (d->mButtonOrientation == Qt::Horizontal)
	{
		static const int layoutRule[5][13] =
		{
			{Details,Help,Default,Stretch,Yes,No,User3,User2,User1,Ok,Apply|Try,Cancel|Close,0},
			{Details,Help,Default,Stretch,Yes,No,User3,User2,User1,Cancel|Close,Apply|Try,Ok,0},
			{Details,Help,Default,Stretch,Yes,No,User3,User2,User1,Apply|Try,Cancel|Close,Ok,0},
			{Ok,Apply|Try,Cancel|Close,User3,User2,User1,No,Yes,Stretch,Default,Help,Details,0},
			{Ok,Cancel|Close,Apply|Try,User3,User2,User1,No,Yes,Stretch,Default,Help,Details,0}
		};
		layout = layoutRule[ d->buttonStyle ];
	}
	else
	{
		static const int layoutRule[5][13] =
		{
			{Ok,Apply|Try,User1,User2,User3,Yes,No,Stretch,Default,Cancel|Close,Help,Details,0},
      //{Ok,Apply|Try,Cancel|Close,User1,User2,User3,Stretch, Default,Help, Details},
			{Details,Help,Default,Stretch,User3,User2,User1,No,Yes,Cancel|Close,Apply|Try,Ok,0},
			{Details,Help,Default,Stretch,User3,User2,User1,No,Yes,Apply|Try,Cancel|Close,Ok,0},
			{Ok,Apply|Try,Cancel|Close,User3,User2,User1,Yes,No,Stretch,Default,Help,Details,0},
			{Ok,Cancel|Close,Apply|Try,User3,User2,User1,Yes,No,Stretch,Default,Help,Details,0}
		};
		layout = layoutRule[ d->buttonStyle ];
	}

	delete d->buttonBox->layout();
	QBoxLayout *lay;
	lay = new QBoxLayout( d->buttonBox, (d->mButtonOrientation == Qt::Horizontal) ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom , 
						  0, spacingHint());

	int numButton = 0;
	QPushButton *prevButton = 0;
	QPushButton *newButton;

	for( int i=0; layout[i]; ++i )
	{
		if(((ButtonCode) layout[i]) == Stretch) // Unconditional Stretch
		{
			lay->addStretch(1);
			continue;
		}
		else if (layout[i] & Filler) // Conditional space
		{
			if (d->buttonMask & layout[i])
			{
				newButton = actionButton( (ButtonCode) (layout[i] & ~(Stretch | Filler)));
				if (newButton)
					lay->addSpacing(newButton->sizeHint().width());
			}
			else
				continue;
		}
		else if( d->buttonMask & layout[i] )
		{
			newButton = actionButton( (ButtonCode)(d->buttonMask & layout[i] & ~(Stretch | Filler)));
			lay->addWidget( newButton );
			++numButton;
		}
		else
			continue;

		// Add conditional stretch (Only added if a button was added)
		if(layout[i] & Stretch)
			lay->addStretch(1);

		if( prevButton )
		{
			setTabOrder( prevButton, newButton );
		}
	    prevButton = newButton;
  	}
	d->resizeButton( false, 0, spacingHint(), d->mButtonOrientation );
	setupLayout();
}



void KDialog::setButtonBoxOrientation( Qt::Orientation orientation )
{
  if( d->mButtonOrientation != orientation )
  {
    d->mButtonOrientation = orientation;
    if( d->mActionSep )
    {
      d->mActionSep->setOrientation( d->mButtonOrientation == Qt::Horizontal ?
				  Qt::Horizontal : Qt::Vertical );
    }
    if( d->mButtonOrientation == Qt::Vertical )
    {
      enableLinkedHelp(false); // 2000-06-18 Espen: No support for this yet.
    }
    setButtonStyle( d->buttonStyle );
  }
}


void KDialog::setEscapeButton( ButtonCode id )
{
  d->mEscapeButton = id;
}

void KDialog::setDefaultButton( ButtonCode defaultButton )
{
  if( defaultButton != NoDefault )
  {
    QPushButton *pb = actionButton( defaultButton );
    if( pb )
    {
      setButtonFocus( pb, true, false );
    }
  }
}


void KDialog::setMainWidget( QWidget *widget )
{
    d->mMainWidget = widget;
      setupLayout();
#if 0
  if( d->mMainWidget != NULL )
  {
    /* There is no more QFocusData in Qt4, so i used QWidget::nextInFocusChain()
       instead - mattr */
    QWidget* prev = nextInFocusChain();

    foreach ( KPushButton* button, d->buttonList )
    {
      if( prev != button )
	setTabOrder( prev, button );
      prev = button;
    }
  }
#endif
}


QWidget *KDialog::mainWidget()
{
  return d->mMainWidget;
}



QSize KDialog::sizeHint() const
{
   return d->minSize.expandedTo( minimumSizeHint() ) + d->incSize;
}

QSize KDialog::minimumSizeHint() const
{
  const int m = marginHint();
  const int s = spacingHint();

  const QSize zeroByS(0, s);

  QSize s1(0,0);
  QSize s2(0,0);

  //
  // Url help area
  //
  if( d->mUrlHelp )
  {
    s2 = d->mUrlHelp->minimumSize() + zeroByS;
  }
  s1.rwidth()   = qMax( s1.rwidth(), s2.rwidth() );
  s1.rheight() += s2.rheight();

  //
  // User widget
  //
  if( d->mMainWidget )
  {
    s2 = d->mMainWidget->sizeHint() + zeroByS;
    s2 = s2.expandedTo( d->mMainWidget->minimumSize() );
    s2 = s2.expandedTo( d->mMainWidget->minimumSizeHint() );
    if( s2.isEmpty() )
    {
      s2 = QSize( 100, 100+s );
    }
  }
  else
  {
    s2 = QSize( 100, 100+s );
  }
  s1.rwidth()  = qMax( s1.rwidth(), s2.rwidth() );
  s1.rheight() += s2.rheight();

  if (d->detailsWidget && d->bDetails)
  {
    s2 = d->detailsWidget->sizeHint() + zeroByS;
    s2 = s2.expandedTo( d->detailsWidget->minimumSize() );
    s2 = s2.expandedTo( d->detailsWidget->minimumSizeHint() );
    s1.rwidth()  = qMax( s1.rwidth(), s2.rwidth() );
    s1.rheight() += s2.rheight();
  }

  //
  // Button separator
  //
  if( d->mActionSep )
  {
    s1.rheight() += d->mActionSep->minimumSize().height() + s;
  }

  //
  // The button box
  //
  if( d->buttonBox )
  {
    s2 = d->buttonBox->minimumSize();
    if( d->mButtonOrientation == Qt::Horizontal )
    {
      s1.rwidth()   = qMax( s1.rwidth(), s2.rwidth() );
      s1.rheight() += s2.rheight();
    }
    else
    {
      s1.rwidth() += s2.rwidth();
      s1.rheight() = qMax( s1.rheight(), s2.rheight() );
    }
  }

  //
  // Outer margings
  //
  s1.rheight() += 2*m;
  s1.rwidth()  += 2*m;

  return s1;
}



//
// Grab QDialogs keypresses if non-modal.
//
void KDialog::keyPressEvent(QKeyEvent *e)
{
  if( e->state() == 0 )
  {
    if( e->key() == Qt::Key_F1 )
    {
      QPushButton *pb = actionButton( Help );
      if( pb )
      {
	pb->animateClick();
	e->accept();
	return;
      }
    }
    if( e->key() == Qt::Key_Escape )
    {
      QPushButton *pb = actionButton( d->mEscapeButton );
      if( pb )
      {
	pb->animateClick();
	e->accept();
	return;
      }

    }
  }
  else if( e->key() == Qt::Key_F1 && e->state() == Qt::ShiftModifier )
  {
    QWhatsThis::enterWhatsThisMode();
    e->accept();
    return;
  }

  // accept the dialog when Ctrl-Return is pressed
  else if ( e->state() == Qt::ControlModifier &&
            (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) )
  {
    QPushButton *pb = actionButton( Ok );
    if ( pb )
    {
      pb->animateClick();
      e->accept();
      return;
    }
  }

  QDialog::keyPressEvent(e);
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
  QDialog::setWindowTitle( caption );

#ifdef Q_WS_X11
  NETWinInfo info( QX11Info::display(), winId(), QX11Info::appRootWindow(), 0 );
  info.setName( caption.toUtf8().data() );
#endif
}


void KDialog::resizeLayout( QWidget *w, int margin, int spacing ) //static
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


void KDialog::resizeLayout( QLayout *lay, int margin, int spacing ) //static
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
      gc.readEntry("XineramaEnabled", true) &&
      gc.readEntry("XineramaPlacementEnabled", true)) {
    if ( screen < 0 || screen >= desktop->numScreens() ) {
      if ( screen == -1 ) {
        screen = desktop->primaryScreen();
      } else if ( screen == -3 ) {
        screen = desktop->screenNumber( QCursor::pos() );
      } else {
        screen = desktop->screenNumber( w );
      }
    }
    return desktop->availableGeometry(screen);
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
  avoid.translate( -5, -5 );
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
  //kDebug() << "Moving window to " << fg.x() << "," << fg.y() << endl;
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

KPushButton *KDialog::appendButton( ButtonCode key, const KGuiItem &item )
{
	KPushButton *p = new KPushButton( item, d->buttonBox );
	d->buttonList.insert( key, p );
	d->buttonSignalMap.setMapping( p , key );
	connect( p, SIGNAL(clicked()), &d->buttonSignalMap, SLOT(map()) );
	return p;
}

void KDialog::Private::resizeButton( bool sameWidth, int margin,
    int spacing, int orientation )
{
  KPushButton *p;
  int h = 0;
  int w = 0;
  int t = 0;

  foreach ( p, buttonList )
  {
    const QSize s( p->sizeHint() );
    if( s.height() > h ) { h = s.height(); }
    if( s.width() > w ) { w = s.width(); }
  }

  if( orientation == Qt::Horizontal )
  {
    foreach ( p, buttonList )
    {
      QSize s( p->sizeHint() );
      if( sameWidth ) { s.setWidth( w ); }
      p->setFixedWidth( s.width() );
      t += s.width() + spacing;
    }

    buttonBox->setMinimumHeight( margin*2 + h );
    buttonBox->setMinimumWidth( margin*2 + t - spacing );
  }
  else
  {
    // sameWidth has no effect here
    foreach ( p, buttonList )
    {
      QSize s( p->sizeHint() );
      s.setWidth( w );
      p->setFixedSize( s );
      t += s.height() + spacing;
    }
    buttonBox->setMinimumHeight( margin*2 + t - spacing );
    buttonBox->setMinimumWidth( margin*2 + w );
  }
}

void KDialog::setupLayout()
{
	delete d->mTopLayout;
	
	d->mTopLayout = (d->mButtonOrientation == Qt::Horizontal) ?
			(QBoxLayout*)new QVBoxLayout( this ) : (QBoxLayout*)new QHBoxLayout( this );

	d->mTopLayout->setMargin(marginHint());
	d->mTopLayout->setSpacing(spacingHint());

	if( d->mUrlHelp )
		d->mTopLayout->addWidget( d->mUrlHelp, 0, Qt::AlignRight );

	if( d->mMainWidget )
		d->mTopLayout->addWidget( d->mMainWidget, 10 );

	if ( d->detailsWidget )
		d->mTopLayout->addWidget( d->detailsWidget );

	if( d->mActionSep )
		d->mTopLayout->addWidget( d->mActionSep );

	if( d->buttonBox )
		d->mTopLayout->addWidget( d->buttonBox );
}


void KDialog::enableButtonSeparator( bool state )
{
	if( state )
	{
		if( d->mActionSep )
			return;
		 d->mActionSep = new KSeparator( this );
		 d->mActionSep->setFocusPolicy(Qt::NoFocus);
		 d->mActionSep->setOrientation( d->mButtonOrientation );
		 d->mActionSep->show();
	}
	else
	{
		if( !d->mActionSep )
			return;
		delete d->mActionSep; 
		d->mActionSep = 0;
	}

	setupLayout();
}




void KDialog::setInitialSize( const QSize &s )
{
  d->minSize = s;
  adjustSize();
}


void KDialog::incInitialSize( const QSize &s )
{
  d->incSize = s;
  adjustSize();
}



KPushButton *KDialog::actionButton( ButtonCode id )
{
  return d->buttonList.contains(id) ? d->buttonList[id] : 0l;
}


void KDialog::enableButton( ButtonCode id, bool state )
{
  QPushButton *pb = actionButton( id );
  if( pb )
  {
    pb->setEnabled( state );
  }
}


void KDialog::enableButtonOK( bool state )
{
  enableButton( Ok, state );
}


void KDialog::enableButtonApply( bool state )
{
  enableButton( Apply, state );
}


void KDialog::enableButtonCancel( bool state )
{
  enableButton( Cancel, state );
}


void KDialog::showButton( ButtonCode id, bool state )
{
  QPushButton *pb = actionButton( id );
  if( pb )
  {
    state ? pb->show() : pb->hide();
  }
}

void KDialog::setButtonGuiItem( ButtonCode id, const KGuiItem &item )
{
  KPushButton *pb = static_cast<KPushButton *>( actionButton( id ) );
  if( !pb )
    return;

  pb->setGuiItem( item );
  d->resizeButton( false, 0, spacingHint(), d->mButtonOrientation );
}


void KDialog::setButtonText( ButtonCode id, const QString &text )
{
  if (!d->bSettingDetails && (id == Details))
  {
    d->detailsButton = text;
    setDetails(d->bDetails);
    return;
  }
  QPushButton *pb = actionButton( id );
  if( pb )
  {
    pb->setText( text );
    d->resizeButton( false, 0, spacingHint(), d->mButtonOrientation );
  }
}


void KDialog::setButtonTip( ButtonCode id, const QString &text )
{
  QPushButton *pb = actionButton( id );
  if( pb )
  {
    if (text.isEmpty())
      pb->setToolTip( QString() );
    else
      pb->setToolTip( text );
  }
}


void KDialog::setButtonWhatsThis( ButtonCode id, const QString &text )
{
  QPushButton *pb = actionButton( id );
  if( pb )
  {
    if (text.isEmpty())
      pb->setWhatsThis(QString());
    else
      pb->setWhatsThis(text );
  }
}


void KDialog::setButtonFocus( QPushButton *p,bool isDefault, bool isFocus )
{
  p->setDefault( isDefault );
  if( isFocus )
      p->setFocus();
}


void KDialog::setDetailsWidget(QWidget *detailsWidget)
{
  delete d->detailsWidget;
  d->detailsWidget = detailsWidget;
  if (d->detailsWidget->parentWidget() != this)
     d->detailsWidget->reparent(this, QPoint(0,0));
  d->detailsWidget->hide();
    setupLayout();
  if (!d->bSettingDetails)
    setDetails(d->bDetails);
}

void KDialog::setDetails(bool showDetails)
{
  if (d->detailsButton.isEmpty())
     d->detailsButton = i18n("&Details");
  d->bSettingDetails = true;
  d->bDetails = showDetails;
  if (d->bDetails)
  {
     emit aboutToShowDetails();
     setButtonText(Details, d->detailsButton+ " <<");
     if (d->detailsWidget)
     {
        if (layout())
           layout()->setEnabled(false);
        adjustSize();
        d->detailsWidget->show();
        if (layout())
        {
           layout()->activate();
           layout()->setEnabled(true);
        }
     }
  }
  else
  {
     setButtonText(Details, d->detailsButton+" >>");
     if (d->detailsWidget)
     {
        d->detailsWidget->hide();
     }
     if (layout())
        layout()->activate();
     adjustSize();
  }
  d->bSettingDetails = false;
}


void KDialog::slotButtonClicked(int button)
{
	emit buttonClicked( Ok );
	switch(button)
	{
		case Ok:
			emit okClicked();
			accept();
			break;
		case Apply:
  			emit applyClicked();
			break;
		case Try:
			emit tryClicked();
			break;
		case User3:
			emit user3Clicked();
			break;
		case User2:
  			emit user2Clicked();
			break;
		case User1:
  			emit user1Clicked();
			break;
		case Yes:
			emit yesClicked();
			done( Yes );
			break;
		case No:
  			emit noClicked();
			done( No );
			break;
		case Cancel:
			emit cancelClicked();
  			reject();
			break;
		case Close:
			emit closeClicked();
  			close();
		case Help:
			emit helpClicked();
			if( !d->mAnchor.isEmpty() || !d->mHelpApp.isEmpty() )
				KToolInvocation::invokeHelp( d->mAnchor, d->mHelpApp );
			break;
		case Default:
			emit defaultClicked();
			break;
		case Details:
			setDetails(!d->bDetails);
			break;
	}
}


void KDialog::enableLinkedHelp( bool state )
{
  if( state )
  {
    if( d->mUrlHelp )
    {
      return;
    }

    d->mUrlHelp = new KUrlLabel( this );
    d->mUrlHelp->setText( helpLinkText() );
    d->mUrlHelp->setFloat(true);
    d->mUrlHelp->setUnderline(true);
    d->mUrlHelp->setMinimumHeight( fontMetrics().height() + marginHint() );
    connect(d->mUrlHelp,SIGNAL(leftClickedURL(const QString &)),
	    SLOT(helpClickedSlot(const QString &)));
    d->mUrlHelp->show();
  }
  else
  {
    if( !d->mUrlHelp )
    {
      return;
    }
    delete d->mUrlHelp; d->mUrlHelp = 0;
  }

    setupLayout();
}


void KDialog::setHelp( const QString &anchor, const QString &appname )
{
  d->mAnchor  = anchor;
  d->mHelpApp = appname;
}


void KDialog::setHelpLinkText( const QString &text )
{
  d->mHelpLinkText = text;
  if( d->mUrlHelp )
  {
    d->mUrlHelp->setText( helpLinkText() );
  }
}


#if 0
QSize KDialog::calculateSize(int w, int h) const
{
  int ulx, uly, lrx, lry;
  ulx = marginHint();
  uly = marginHint();
  if( d->mUrlHelp  )
  {
    uly += d->mUrlHelp->minimumSize().height();
  }

  lrx = marginHint();
  lry = d->buttonBox ? d->buttonBox->minimumSize().height() : 0;
  if( d->mActionSep )
  {
    lry += d->mActionSep->minimumSize().height() + marginHint();
  }
  return QSize(ulx+w+lrx,uly+h+lry);
}
#endif


QString KDialog::helpLinkText() const
{
  return ( d->mHelpLinkText.isNull() ? i18n("Get help...") : d->mHelpLinkText );
}


void KDialog::updateGeometry()
{
  if( d->mTopLayout )
  {
    d->mTopLayout->setMargin( marginHint() );
    d->mTopLayout->setSpacing(spacingHint() );
  }
}





void KDialog::hideEvent( QHideEvent *ev )
{
  emit hidden();
  if (!ev->spontaneous())
  {
     emit finished();
  }
}



void KDialog::closeEvent( QCloseEvent *e )
{
    QPushButton *pb = actionButton( d->mEscapeButton );
    if( pb && !isHidden() ) {
	pb->animateClick();
    } else {
	QDialog::closeEvent( e );
    }
}



void KDialog::restoreDialogSize( KConfigBase *cfg )
{
	int w, h;
	int scnum = QApplication::desktop()->screenNumber(parentWidget());
	QRect desk = QApplication::desktop()->screenGeometry(scnum);
	
	w = sizeHint().width();
	h = sizeHint().height();
	
	w = cfg->readEntry( QString::fromLatin1("Width %1").arg( desk.width()), w );
	h = cfg->readEntry( QString::fromLatin1("Height %1").arg( desk.height()), h );
	
	resize(w,h);
}




void KDialog::saveDialogSize( KConfigBase* config, KConfigBase::WriteConfigFlags options ) const
{
   int scnum = QApplication::desktop()->screenNumber(parentWidget());
   QRect desk = QApplication::desktop()->screenGeometry(scnum);

   QSize sizeToSave = size();

   config->writeEntry( QString::fromLatin1("Width %1").arg( desk.width()),  sizeToSave.width(), options   );
   config->writeEntry( QString::fromLatin1("Height %1").arg( desk.height()),  sizeToSave.height(), options );
}



void KDialog::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdialog.moc"
