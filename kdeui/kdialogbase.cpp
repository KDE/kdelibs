/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2001 Mirko Boehm (mirko@kde.org) and
 *  Espen Sand (espen@kde.org)
 *  Holger Freyther <freyther@kde.org>
 *  2005-2006 Olivier Goffart <ogoffart at kde.org>
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
 *
 */

#include "kdialogbase.h"
#include <stdlib.h>

#ifdef KDE3_SUPPORT
#include <q3grid.h>
#endif

#include <qlayout.h>
#include <qtimer.h>
#include <QKeyEvent>
#include <QHideEvent>
#include <QDesktopWidget>
#include <QWhatsThis>
#include <QApplication>
#include <QPushButton>

#include <ktoolinvocation.h>
#include <klocale.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kseparator.h>
#include <kurllabel.h>
#include <kdebug.h>
#include <khbox.h>
#include <kvbox.h>

#include "kdialogbase_priv.h"
#include "kdialogbase_priv.moc"

KDialogBaseTile *KDialogBase::mTile = 0;

class KDialogBase::KDialogBasePrivate {
public:
    KDialogBasePrivate() : bFixed(false) { }

    bool bFixed;
    QSize incSize;
    QSize minSize;
};

KDialogBase::KDialogBase( QWidget *parent, const char *name, bool modal,
			  const QString &caption, int buttonMask,
			  ButtonCode defaultButton, bool separator,
			  const KGuiItem &user1, const KGuiItem &user2,
			  const KGuiItem &user3 )
  :KDialog( parent, caption , (ButtonCode)buttonMask , Qt::WStyle_DialogBorder , user1, user2, user3 ),
   mJanus(0) ,  mShowTile(false), d(new KDialogBasePrivate)
{
  setObjectName( name );
  setDefaultButton( defaultButton );
  enableButtonSeparator( separator );
  setModal( modal );

  makeRelay();
}

KDialogBase::KDialogBase( int dialogFace, const QString &caption,
			  int buttonMask, ButtonCode defaultButton,
			  QWidget *parent, const char *name, bool modal,
			  bool separator, const KGuiItem &user1,
			  const KGuiItem &user2, const KGuiItem &user3 )
  :KDialog( parent, caption , (ButtonCode)buttonMask , Qt::WStyle_DialogBorder , user1, user2, user3 ),
   mJanus(0) ,  mShowTile(false), d(new KDialogBasePrivate)
{
  setObjectName( name );
  setDefaultButton( defaultButton );
  enableButtonSeparator( separator );
  setModal( modal );

  makeRelay();

  mJanus = new KJanusWidget( this, dialogFace );
  connect(mJanus, SIGNAL(currentPageChanged(QWidget *)),
          this, SIGNAL(currentPageChanged(QWidget *)));

  if( !mJanus || !mJanus->isValid() ) { return; }

  setMainWidget( mJanus );
}

KDialogBase::KDialogBase(  KDialogBase::DialogType dialogFace, Qt::WFlags f, QWidget *parent, const char *name,
		          bool modal,
			  const QString &caption, int buttonMask,
			  ButtonCode defaultButton, bool separator,
			  const KGuiItem &user1, const KGuiItem &user2,
			  const KGuiItem &user3 )
  :KDialog( parent, caption , (ButtonCode)buttonMask , f , user1, user2, user3 ),
   mJanus(0) ,  mShowTile(false), d(new KDialogBasePrivate)
{
  setObjectName( name );
  setDefaultButton( defaultButton );
  enableButtonSeparator( separator );
  setModal( modal );


  makeRelay();

  mJanus = new KJanusWidget( this, dialogFace );
  connect(mJanus, SIGNAL(currentPageChanged(QWidget *)),
          this, SIGNAL(currentPageChanged(QWidget *)));

  if( !mJanus || !mJanus->isValid() ) { return; }

  setMainWidget( mJanus );

}

KDialogBase::KDialogBase( const QString &caption, int buttonMask,
			  ButtonCode defaultButton, ButtonCode escapeButton,
			  QWidget *parent, const char *name, bool modal,
			  bool separator, const KGuiItem &yes,
			  const KGuiItem &no, const KGuiItem &cancel )
  :KDialog( parent, caption ,(ButtonCode) buttonMask , Qt::WStyle_DialogBorder ),
   mJanus(0) ,  mShowTile(false), d(new KDialogBasePrivate)
{
  setObjectName( name );
  setDefaultButton( defaultButton );
  setEscapeButton( escapeButton );
  enableButtonSeparator( separator );
  setModal( modal );
  setButtonGuiItem(Yes, yes.text().isEmpty() ? KStdGuiItem::yes() : yes);
  setButtonGuiItem(No, no.text().isEmpty()  ? KStdGuiItem::no()  : no);
  setButtonGuiItem(Cancel, cancel.text().isEmpty() ? KStdGuiItem::cancel() : cancel);

  makeRelay();
}



KDialogBase::~KDialogBase()
{
  delete d;
}


void
KDialogBase::delayedDestruct()
{
   if (isVisible())
      hide();
   QTimer::singleShot( 0, this, SLOT(slotDelayedDestruct()));
}

void
KDialogBase::slotDelayedDestruct()
{
   delete this;
}



void KDialogBase::makeRelay()
{

	connect(this, SIGNAL(applyClicked()) , this, SLOT(slotApply()));

  if( mTile )
  {
    connect( mTile, SIGNAL(pixmapChanged()), SLOT(updateBackground()) );
    return;
  }

  mTile = new KDialogBaseTile;
  if( mTile )
  {
    connect( mTile, SIGNAL(pixmapChanged()), SLOT(updateBackground()) );
    connect( qApp, SIGNAL(aboutToQuit()), mTile, SLOT(cleanup()) );
  }
}


QFrame *KDialogBase::plainPage()
{
	return ( mJanus ? mJanus->plainPage() : 0 );
}



void KDialogBase::disableResize()
{
  setFixedSize( sizeHint() );
}


void KDialogBase::showButtonOK( bool state )
{
  showButton( Ok, state );
}


void KDialogBase::showButtonApply( bool state )
{
  showButton( Apply, state );
}


void KDialogBase::showButtonCancel( bool state )
{
  showButton( Cancel, state );
}


void KDialogBase::setButtonOK( const KGuiItem &item /* = KStdGuiItem::ok() */ )
{
  setButtonGuiItem( Ok, item );
}


void KDialogBase::setButtonApply( const KGuiItem &item /* = KStdGuiItem::apply() */ )
{
  setButtonGuiItem( Apply, item );
}


void KDialogBase::setButtonCancel( const KGuiItem &item /* = KStdGuiItem::cancel() */ )
{
  setButtonGuiItem( Cancel, item );
}

void KDialogBase::setTreeListAutoResize( bool state )
{
  if( mJanus )
  {
    mJanus->setTreeListAutoResize( state );
  }
}

void KDialogBase::setShowIconsInTreeList(bool state)
{
  if( mJanus )
  {
    mJanus->setShowIconsInTreeList( state );
  }
}

void KDialogBase::setRootIsDecorated( bool state )
{
  if( mJanus )
  {
    mJanus->setRootIsDecorated( state );
  }
}

void KDialogBase::unfoldTreeList( bool persist )
{
  if( mJanus )
    mJanus->unfoldTreeList( persist );
}

void KDialogBase::addWidgetBelowList( QWidget * widget )
{
  if( mJanus )
    mJanus->addWidgetBelowList( widget );
}

void KDialogBase::addButtonBelowList( const QString & text, QObject * recv, const char * slot )
{
  if( mJanus )
    mJanus->addButtonBelowList( text, recv, slot );
}

void KDialogBase::addButtonBelowList( const KGuiItem & item, QObject * recv, const char * slot )
{
  if( mJanus )
    mJanus->addButtonBelowList( item, recv, slot );
}

void KDialogBase::setIconListAllVisible( bool state )
{
  if( mJanus )
  {
    mJanus->setIconListAllVisible( state );
  }
}



QFrame *KDialogBase::addPage( const QString &itemName, const QString &header,
			      const QPixmap &pixmap )
{
  return ( mJanus ? mJanus->addPage( itemName, header, pixmap ) : 0);
}

QFrame *KDialogBase::addPage( const QStringList &items, const QString &header,
			      const QPixmap &pixmap )
{
  return ( mJanus ? mJanus->addPage( items, header, pixmap ) : 0);
}


KVBox *KDialogBase::addVBoxPage( const QString &itemName,
				 const QString &header, const QPixmap &pixmap )
{
  return ( mJanus ? mJanus->addVBoxPage( itemName, header, pixmap) : 0);
}

KVBox *KDialogBase::addVBoxPage( const QStringList &items,
				 const QString &header, const QPixmap &pixmap )
{
  return ( mJanus ? mJanus->addVBoxPage( items, header, pixmap) : 0);
}


KHBox *KDialogBase::addHBoxPage( const QString &itemName,
				 const QString &header,
				 const QPixmap &pixmap )
{
  return ( mJanus ? mJanus->addHBoxPage( itemName, header, pixmap ) : 0);
}

KHBox *KDialogBase::addHBoxPage( const QStringList &items,
				 const QString &header,
				 const QPixmap &pixmap )
{
  return ( mJanus ? mJanus->addHBoxPage( items, header, pixmap ) : 0);
}

#ifdef KDE3_SUPPORT
Q3Grid *KDialogBase::addGridPage( int n, Qt::Orientation dir,
				 const QString &itemName,
				 const QString &header, const QPixmap &pixmap )
{
  return ( mJanus ? mJanus->addGridPage( n, dir, itemName, header, pixmap) : 0);
}

Q3Grid *KDialogBase::addGridPage( int n, Qt::Orientation dir,
				 const QStringList &items,
				 const QString &header, const QPixmap &pixmap )
{
  return ( mJanus ? mJanus->addGridPage( n, dir, items, header, pixmap) : 0);
}
#endif

void KDialogBase::setFolderIcon(const QStringList &path, const QPixmap &pixmap)
{
  if (!mJanus)
    return;

  mJanus->setFolderIcon(path,pixmap);
}

QFrame *KDialogBase::makeMainWidget()
{
  if( mJanus  )
  {
    printMakeMainWidgetError();
    return 0;
  }

  QFrame *mainWidget = new QFrame( this );
  setMainWidget( mainWidget );
  return mainWidget;
}


KVBox *KDialogBase::makeVBoxMainWidget()
{
  if( mJanus  )
  {
    printMakeMainWidgetError();
    return 0;
  }

  KVBox *mainWidget = new KVBox( this );
  mainWidget->setSpacing( spacingHint() );
  setMainWidget( mainWidget );
  return mainWidget;
}


KHBox *KDialogBase::makeHBoxMainWidget()
{
  if( mJanus  )
  {
    printMakeMainWidgetError();
    return 0;
  }

  KHBox *mainWidget = new KHBox( this );
  mainWidget->setSpacing( spacingHint() );
  setMainWidget( mainWidget );
  return mainWidget;
}

#ifdef KDE3_SUPPORT
Q3Grid *KDialogBase::makeGridMainWidget( int n, Qt::Orientation dir )
{
  if( mJanus  )
  {
    printMakeMainWidgetError();
    return 0;
  }

  Q3Grid *mainWidget = new Q3Grid( n, dir, this );
  mainWidget->setSpacing( spacingHint() );
  setMainWidget( mainWidget );
  return mainWidget;
}
#endif

void KDialogBase::printMakeMainWidgetError()
{
  if( mJanus )
  {
    kdDebug() << "makeMainWidget: Illegal mode (wrong constructor)" << endl;
  }
}


void KDialogBase::setMainWidget( QWidget *widget )
{
  if( mJanus )
  {
    mJanus->setSwallowedWidget(widget);
  }
  else KDialog::setMainWidget( widget );
}


bool KDialogBase::showPage( int index )
{
  return ( mJanus ? mJanus->showPage(index) : false);
}


int KDialogBase::activePageIndex() const
{
  return ( mJanus ? mJanus->activePageIndex() : -1);
}


int KDialogBase::pageIndex( QWidget *widget ) const
{
  return ( mJanus ? mJanus->pageIndex( widget) : -1);
}



QSize KDialogBase::configDialogSize( const QString& groupName ) const
{
   return configDialogSize( *KGlobal::config(), groupName );
}


QSize KDialogBase::configDialogSize( KConfig& config,
				      const QString& groupName ) const
{
   int w, h;
   int scnum = QApplication::desktop()->screenNumber(parentWidget());
   QRect desk = QApplication::desktop()->screenGeometry(scnum);

   w = sizeHint().width();
   h = sizeHint().height();

   KConfigGroup cg(&config, groupName);
   w = cg.readEntry( QString::fromLatin1("Width %1").arg( desk.width()), w );
   h = cg.readEntry( QString::fromLatin1("Height %1").arg( desk.height()), h );

   return size( );
}


void KDialogBase::saveDialogSize( const QString& groupName,  bool global )
{
	KConfigGroup cg( KGlobal::config() , groupName);
	KDialog::saveDialogSize(&cg , global ? KConfigBase::Global : KConfigBase::Normal );
}
void KDialogBase::saveDialogSize( KConfig& config, const QString& groupName,  bool global ) const
{
	KConfigGroup cg( &config , groupName);
	KDialog::saveDialogSize(&cg , global ? KConfigBase::Global : KConfigBase::Normal );
}






bool KDialogBase::haveBackgroundTile()
{
  return ( !mTile || mTile->get() );
}

const QPixmap *KDialogBase::backgroundTile()
{
  return ( mTile ? mTile->get() : 0);
}


void KDialogBase::setBackgroundTile( const QPixmap *pix )
{
  if( mTile )
  {
    mTile->set( pix );
  }
}


void KDialogBase::updateBackground()
{
  if( !mTile || !mTile->get() )
  {
    QPixmap nullPixmap;
    setBackgroundPixmap(nullPixmap);
#if 0
    if( d->mButton.box )
    {
      d->mButton.box->setBackgroundPixmap(nullPixmap);
      d->mButton.box->setBackgroundMode(Qt::PaletteBackground);
    }
#endif
    setBackgroundMode(Qt::PaletteBackground);
  }
  else
  {
    const QPixmap *pix = mTile->get();
    setBackgroundPixmap(*pix);
#if 0
    if( d->mButton.box )
    {
      d->mButton.box->setBackgroundPixmap(*pix);
    }
#endif
    showTile( mShowTile );
  }
}


void KDialogBase::showTile( bool state )
{
  mShowTile = state;
  if( !mShowTile || !mTile || !mTile->get() )
  {
    setBackgroundMode(Qt::PaletteBackground);
#if 0
    if( d->mButton.box )
    {
      d->mButton.box->setBackgroundMode(Qt::PaletteBackground);
    }
    if( mUrlHelp )
    {
      mUrlHelp->setBackgroundMode(Qt::PaletteBackground);
    }
#endif
  }
  else
  {
    const QPixmap *pix = mTile->get();
    setBackgroundPixmap(*pix);
#if 0
    if( d->mButton.box )
    {
      d->mButton.box->setBackgroundPixmap(*pix);
    }
    if( mUrlHelp )
    {
      mUrlHelp->setBackgroundPixmap(*pix);
    }
#endif
  }
}


KDialogBaseTile::KDialogBaseTile( QObject *parent )
  : QObject( parent ), mPixmap(0)
{
}


KDialogBaseTile::~KDialogBaseTile()
{
  cleanup();
}


void KDialogBaseTile::set( const QPixmap *pix )
{
  if( !pix )
  {
    cleanup();
  }
  else
  {
    if( !mPixmap )
    {
      mPixmap = new QPixmap(*pix);
    }
    else
    {
      *mPixmap = *pix;
    }
  }

  emit pixmapChanged();
}


const QPixmap *KDialogBaseTile::get() const
{
  return mPixmap;
}


void KDialogBaseTile::cleanup()
{
  delete mPixmap; mPixmap = 0;
}


void KDialogBase::virtual_hook( int id, void* data )
{ KDialog::virtual_hook( id, data ); }


//This is done in order to keep the same behavior as in KDE3: 
//  slotOk may call accept,  so we need to call slotOk only if accept() is called because the button Ok has been pressed
void KDialogBase::accept() 
{ 
	if(sender()->inherits("QSignalMapper"))  QTimer::singleShot(0,this,SLOT(slotOk()));
	else KDialog::accept(); 
}
void KDialogBase::slotOk() { KDialog::accept(); }
void KDialogBase::reject()
{ 
	if(sender()->inherits("QSignalMapper"))  QTimer::singleShot(0,this,SLOT(slotCancel()));
	else KDialog::reject(); 
}
void KDialogBase::slotCancel() { KDialog::reject(); }


#include "kdialogbase.moc"
