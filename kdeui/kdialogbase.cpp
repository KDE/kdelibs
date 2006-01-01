/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2001 Mirko Boehm (mirko@kde.org) and
 *  Espen Sand (espen@kde.org)
 *  Holger Freyther <freyther@kde.org>
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

int KDialogBaseButton::id()
{
    return mKey;
}

/**
 * @internal
 */
namespace
{
struct SButton
{
  SButton()
  {
    box = 0;
    mask = 0;
    style = 0;
  }

  KPushButton *append( int key, const KGuiItem &item );

  void resize( bool sameWidth, int margin, int spacing, int orientation );

  KPushButton *button( int key );

  QWidget *box;
  int mask;
  int style;
  QList<KDialogBaseButton*> list;
};
}// namespace

class KDialogBase::KDialogBasePrivate {
public:
    KDialogBasePrivate() : bDetails(false), bFixed(false), bSettingDetails(false), detailsWidget(0) { }

    bool bDetails;
    bool bFixed;
    bool bSettingDetails;
    QWidget *detailsWidget;
    QSize incSize;
    QSize minSize;
    QString detailsButton;
    SButton mButton;
};

KDialogBase::KDialogBase( QWidget *parent, const char *name, bool modal,
			  const QString &caption, int buttonMask,
			  ButtonCode defaultButton, bool separator,
			  const KGuiItem &user1, const KGuiItem &user2,
			  const KGuiItem &user3 )
  :KDialog( parent, name, modal, Qt::WStyle_DialogBorder ),
   mTopLayout(0), mMainWidget(0), mUrlHelp(0), mJanus(0), mActionSep(0),
   mIsActivated(false), mShowTile(false), mMessageBoxMode(false),
   mButtonOrientation(Qt::Horizontal), d(new KDialogBasePrivate)
{
  setCaption( caption );

  makeRelay();
  connect( this, SIGNAL(layoutHintChanged()), SLOT(updateGeometry()) );

  enableButtonSeparator( separator );
  makeButtonBox( buttonMask, defaultButton, user1, user2, user3 );

  mIsActivated = true;
  setupLayout();
}

KDialogBase::KDialogBase( int dialogFace, const QString &caption,
			  int buttonMask, ButtonCode defaultButton,
			  QWidget *parent, const char *name, bool modal,
			  bool separator, const KGuiItem &user1,
			  const KGuiItem &user2, const KGuiItem &user3 )
  :KDialog( parent, name, modal, Qt::WStyle_DialogBorder ),
   mTopLayout(0), mMainWidget(0), mUrlHelp(0), mJanus(0), mActionSep(0),
   mIsActivated(false), mShowTile(false), mMessageBoxMode(false),
   mButtonOrientation(Qt::Horizontal), d(new KDialogBasePrivate)
{
  setCaption( caption );

  makeRelay();
  connect( this, SIGNAL(layoutHintChanged()), SLOT(updateGeometry()) );

  mJanus = new KJanusWidget( this, dialogFace );
  connect(mJanus, SIGNAL(currentPageChanged(QWidget *)),
          this, SIGNAL(currentPageChanged(QWidget *)));

  if( !mJanus || !mJanus->isValid() ) { return; }

  enableButtonSeparator( separator );
  makeButtonBox( buttonMask, defaultButton, user1, user2, user3 );

  mIsActivated = true;
  setupLayout();
}

KDialogBase::KDialogBase(  KDialogBase::DialogType dialogFace, Qt::WFlags f, QWidget *parent, const char *name,
		          bool modal,
			  const QString &caption, int buttonMask,
			  ButtonCode defaultButton, bool separator,
			  const KGuiItem &user1, const KGuiItem &user2,
			  const KGuiItem &user3 )
  :KDialog( parent, name, modal, f ),
   mTopLayout(0), mMainWidget(0), mUrlHelp(0), mJanus(0), mActionSep(0),
   mIsActivated(false), mShowTile(false), mMessageBoxMode(false),
   mButtonOrientation(Qt::Horizontal), d(new KDialogBasePrivate)
{
  setCaption( caption );

  makeRelay();
  connect( this, SIGNAL(layoutHintChanged()), SLOT(updateGeometry()) );

  mJanus = new KJanusWidget( this, dialogFace );
  connect(mJanus, SIGNAL(currentPageChanged(QWidget *)),
          this, SIGNAL(currentPageChanged(QWidget *)));

  if( !mJanus || !mJanus->isValid() ) { return; }

  enableButtonSeparator( separator );
  makeButtonBox( buttonMask, defaultButton, user1, user2, user3 );

  mIsActivated = true;
  setupLayout();
}

KDialogBase::KDialogBase( const QString &caption, int buttonMask,
			  ButtonCode defaultButton, ButtonCode escapeButton,
			  QWidget *parent, const char *name, bool modal,
			  bool separator, const KGuiItem &yes,
			  const KGuiItem &no, const KGuiItem &cancel )
  :KDialog( parent, name, modal, Qt::WStyle_DialogBorder ),
   mTopLayout(0), mMainWidget(0), mUrlHelp(0), mJanus(0), mActionSep(0),
   mIsActivated(false), mShowTile(false), mMessageBoxMode(true),
   mButtonOrientation(Qt::Horizontal),mEscapeButton(escapeButton),
   d(new KDialogBasePrivate)
{
  setCaption( caption );

  makeRelay();
  connect( this, SIGNAL(layoutHintChanged()), SLOT(updateGeometry()) );

  enableButtonSeparator( separator );

  buttonMask &= Details|Yes|No|Cancel;

  makeButtonBox( buttonMask, defaultButton,
                 no.text().isEmpty()  ? KStdGuiItem::no()  : no,
                 yes.text().isEmpty() ? KStdGuiItem::yes() : yes );

  setButtonCancel( cancel.text().isEmpty() ?
                       KStdGuiItem::cancel() : cancel );

  mIsActivated = true;
  setupLayout();
}



KDialogBase::~KDialogBase()
{
  delete d;
}

KPushButton *SButton::append( int key, const KGuiItem &item )
{
  KDialogBaseButton *p = new KDialogBaseButton( item, key, box );
  list.append( p );
  return p;
}

void SButton::resize( bool sameWidth, int margin,
    int spacing, int orientation )
{
  KDialogBaseButton *p;
  int h = 0;
  int w = 0;
  int t = 0;

  foreach ( p, list )
  {
    const QSize s( p->sizeHint() );
    if( s.height() > h ) { h = s.height(); }
    if( s.width() > w ) { w = s.width(); }
  }

  if( orientation == Qt::Horizontal )
  {
    foreach ( p, list )
    {
      QSize s( p->sizeHint() );
      if( sameWidth ) { s.setWidth( w ); }
      p->setFixedWidth( s.width() );
      t += s.width() + spacing;
    }

    box->setMinimumHeight( margin*2 + h );
    box->setMinimumWidth( margin*2 + t - spacing );
  }
  else
  {
    // sameWidth has no effect here
    foreach ( p, list )
    {
      QSize s( p->sizeHint() );
      s.setWidth( w );
      p->setFixedSize( s );
      t += s.height() + spacing;
    }
    box->setMinimumHeight( margin*2 + t - spacing );
    box->setMinimumWidth( margin*2 + w );
  }
}

KPushButton *SButton::button( int key )
{
  foreach ( KDialogBaseButton *p, list )
  {
    if( p->id() == key )
    {
      return p;
    }
  }
  return 0;
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

void KDialogBase::setupLayout()
{
  if( mTopLayout )
  {
    delete mTopLayout;
  }
  // mTopLayout = new QVBoxLayout( this, marginHint(), spacingHint() );


  if( mButtonOrientation == Qt::Horizontal )
  {
    mTopLayout = new QVBoxLayout( this );
    mTopLayout->setMargin(marginHint());
    mTopLayout->setSpacing(spacingHint());
  }
  else
  {
    mTopLayout = new QHBoxLayout( this);
    mTopLayout->setMargin(marginHint());
    mTopLayout->setSpacing(spacingHint());
  }

  if( mUrlHelp )
  {
    mTopLayout->addWidget( mUrlHelp, 0, Qt::AlignRight );
  }

  if( mJanus )
  {
    mTopLayout->addWidget( mJanus, 10 );
  }
  else if( mMainWidget )
  {
    mTopLayout->addWidget( mMainWidget, 10 );
  }

  if ( d->detailsWidget )
  {
    mTopLayout->addWidget( d->detailsWidget );
  }

  if( mActionSep )
  {
    mTopLayout->addWidget( mActionSep );
  }

  if( d->mButton.box )
  {
    mTopLayout->addWidget( d->mButton.box );
  }
}



void KDialogBase::setButtonBoxOrientation( int orientation )
{
  if( mButtonOrientation != orientation )
  {
    mButtonOrientation = orientation;
    if( mActionSep )
    {
      mActionSep->setOrientation( mButtonOrientation == Qt::Horizontal ?
				  Qt::Horizontal : Qt::Vertical );
    }
    if( mButtonOrientation == Qt::Vertical )
    {
      enableLinkedHelp(false); // 2000-06-18 Espen: No support for this yet.
    }
    setupLayout();
    setButtonStyle( d->mButton.style );
  }
}


void KDialogBase::setEscapeButton( ButtonCode id )
{
  mEscapeButton = id;
}



void KDialogBase::makeRelay()
{
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


void KDialogBase::enableButtonSeparator( bool state )
{
  if( state )
  {
    if( mActionSep )
    {
      return;
    }
    mActionSep = new KSeparator( this );
    mActionSep->setFocusPolicy(Qt::NoFocus);
    mActionSep->setOrientation( mButtonOrientation == Qt::Horizontal ?
				Qt::Horizontal : Qt::Vertical );
    mActionSep->show();
  }
  else
  {
    if( !mActionSep )
    {
      return;
    }
    delete mActionSep; mActionSep = 0;
  }

  if( mIsActivated )
  {
    setupLayout();
  }
}



QFrame *KDialogBase::plainPage()
{
	return ( mJanus ? mJanus->plainPage() : 0 );
}



void KDialogBase::adjustSize()
{
//  if (layout())
//     layout()->activate();
  if( d->bFixed )
    setFixedSize( sizeHint() );
  else
    resize( sizeHint() );
}

QSize KDialogBase::sizeHint() const
{
   return d->minSize.expandedTo( minimumSizeHint() ) + d->incSize;
}

QSize KDialogBase::minimumSizeHint() const
{
  const int m = marginHint();
  const int s = spacingHint();

  const QSize zeroByS(0, s);

  QSize s1(0,0);
  QSize s2(0,0);

  //
  // Url help area
  //
  if( mUrlHelp )
  {
    s2 = mUrlHelp->minimumSize() + zeroByS;
  }
  s1.rwidth()   = qMax( s1.rwidth(), s2.rwidth() );
  s1.rheight() += s2.rheight();

  //
  // User widget
  //
  if( mJanus )
  {
    s2 = mJanus->minimumSizeHint() + zeroByS;
  }
  else if( mMainWidget )
  {
    s2 = mMainWidget->sizeHint() + zeroByS;
    s2 = s2.expandedTo( mMainWidget->minimumSize() );
    s2 = s2.expandedTo( mMainWidget->minimumSizeHint() );
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
  if( mActionSep )
  {
    s1.rheight() += mActionSep->minimumSize().height() + s;
  }

  //
  // The button box
  //
  if( d->mButton.box )
  {
    s2 = d->mButton.box->minimumSize();
    if( mButtonOrientation == Qt::Horizontal )
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


void KDialogBase::disableResize()
{
  setFixedSize( sizeHint() );
}


void KDialogBase::setInitialSize( const QSize &s, bool noResize )
{
  d->minSize = s;
  d->bFixed = noResize;
  adjustSize();
}


void KDialogBase::incInitialSize( const QSize &s, bool noResize )
{
  d->incSize = s;
  d->bFixed = noResize;
  adjustSize();
}


void KDialogBase::makeButtonBox( int buttonMask, ButtonCode defaultButton,
				 const KGuiItem &user1, const KGuiItem &user2,
				 const KGuiItem &user3 )
{
  if( buttonMask == 0 )
  {
    d->mButton.box = 0;
    return; // When we want no button box
  }

  if( buttonMask & Cancel ) { buttonMask &= ~Close; }
  if( buttonMask & Apply ) { buttonMask &= ~Try; }
  if( buttonMask & Details ) { buttonMask &= ~Default; }

  if( !mMessageBoxMode )
  {
    mEscapeButton = (buttonMask&Cancel) ? Cancel : Close;
  }

  d->mButton.box = new QWidget( this );

  d->mButton.mask = buttonMask;
  if( d->mButton.mask & Help )
  {
    KPushButton *pb = d->mButton.append( Help, KStdGuiItem::help() );

    connect( pb, SIGNAL(clicked()), SLOT(slotHelp()) );
  }
  if( d->mButton.mask & Default )
  {
    KPushButton *pb = d->mButton.append( Default, KStdGuiItem::defaults() );

    connect( pb, SIGNAL(clicked()), SLOT(slotDefault()) );
  }
  if( d->mButton.mask & Details )
  {
    KPushButton *pb = d->mButton.append( Details, QString() );
    connect( pb, SIGNAL(clicked()), SLOT(slotDetails()) );
    setDetails(false);
  }
  if( d->mButton.mask & User3 )
  {
    KPushButton *pb = d->mButton.append( User3, user3 );
    connect( pb, SIGNAL(clicked()), SLOT(slotUser3()) );
  }
  if( d->mButton.mask & User2 )
  {
    KPushButton *pb = d->mButton.append( User2, user2 );
    if( mMessageBoxMode )
    {
      connect( pb, SIGNAL(clicked()), SLOT(slotYes()) );
    }
    else
    {
      connect( pb, SIGNAL(clicked()), this, SLOT(slotUser2()) );
    }
  }
  if( d->mButton.mask & User1 )
  {
    KPushButton *pb = d->mButton.append( User1, user1 );
    if( mMessageBoxMode )
    {
      connect( pb, SIGNAL(clicked()), this, SLOT(slotNo()) );
    }
    else
    {
      connect( pb, SIGNAL(clicked()), SLOT(slotUser1()) );
    }
  }
  if( d->mButton.mask & Ok )
  {
    KPushButton *pb = d->mButton.append( Ok, KStdGuiItem::ok() );
    connect( pb, SIGNAL(clicked()), SLOT(slotOk()) );
  }
  if( d->mButton.mask & Apply )
  {
    KPushButton *pb = d->mButton.append( Apply, KStdGuiItem::apply() );
    connect( pb, SIGNAL(clicked()), SLOT(slotApply()) );
    connect( pb, SIGNAL(clicked()), SLOT(applyPressed()) );
  }
  if( d->mButton.mask & Try )
  {
    KPushButton *pb = d->mButton.append( Try,
                           i18n( "&Try" ) );
    connect( pb, SIGNAL(clicked()), SLOT(slotTry()) );
  }
  if( d->mButton.mask & Cancel )
  {
    KPushButton *pb = d->mButton.append( Cancel, KStdGuiItem::cancel() );
    connect( pb, SIGNAL(clicked()), SLOT(slotCancel()) );
  }
  if( d->mButton.mask & Close )
  {
    KPushButton *pb = d->mButton.append( Close, KStdGuiItem::close() );
    connect( pb, SIGNAL(clicked()), SLOT(slotClose()) );
  }

  if( defaultButton != NoDefault )
  {
    QPushButton *pb = actionButton( defaultButton );
    if( pb )
    {
      setButtonFocus( pb, true, false );
    }
  }

  setButtonStyle( KGlobalSettings::buttonLayout() );
}



void KDialogBase::setButtonStyle( int style )
{
  if( !d->mButton.box )
  {
    return;
  }

  if( style < 0 || style > ActionStyleMAX ) { style = ActionStyle0; }
  d->mButton.style = style;

  const int *layout;
  int layoutMax = 0;
  if (mMessageBoxMode)
  {
    static const int layoutRule[5][6] =
    {
      {Details,Stretch,User2|Stretch,User1|Stretch,Cancel|Stretch, Details|Filler},
      {Details,Stretch,User2|Stretch,User1|Stretch,Cancel|Stretch, Details|Filler},
      {Details,Stretch,User2|Stretch,User1|Stretch,Cancel|Stretch, Details|Filler},
      {Details|Filler,Stretch,Cancel|Stretch,User2|Stretch,User1|Stretch,Details},
      {Details|Filler,Stretch,Cancel|Stretch,User2|Stretch,User1|Stretch,Details}
    };
    layoutMax = 6;
    layout = layoutRule[ d->mButton.style ];
  }
  else if (mButtonOrientation == Qt::Horizontal)
  {
    static const int layoutRule[5][10] =
    {
      {Details,Help,Default,Stretch,User3,User2,User1,Ok,Apply|Try,Cancel|Close},
      {Details,Help,Default,Stretch,User3,User2,User1,Cancel|Close,Apply|Try,Ok},
      {Details,Help,Default,Stretch,User3,User2,User1,Apply|Try,Cancel|Close,Ok},
      {Ok,Apply|Try,Cancel|Close,User3,User2,User1,Stretch,Default,Help,Details},
      {Ok,Cancel|Close,Apply|Try,User3,User2,User1,Stretch,Default,Help,Details}
    };
    layoutMax = 10;
    layout = layoutRule[ d->mButton.style ];
  }
  else
  {
    static const int layoutRule[5][10] =
    {
      {Ok,Apply|Try,User1,User2,User3,Stretch,Default,Cancel|Close,Help, Details},
      //{Ok,Apply|Try,Cancel|Close,User1,User2,User3,Stretch, Default,Help, Details},
      {Details,Help,Default,Stretch,User3,User2,User1,Cancel|Close,Apply|Try,Ok},
      {Details,Help,Default,Stretch,User3,User2,User1,Apply|Try,Cancel|Close,Ok},
      {Ok,Apply|Try,Cancel|Close,User3,User2,User1,Stretch,Default,Help,Details},
      {Ok,Cancel|Close,Apply|Try,User3,User2,User1,Stretch,Default,Help,Details}
    };
    layoutMax = 10;
    layout = layoutRule[ d->mButton.style ];
  }

  if( d->mButton.box->layout() )
  {
    delete d->mButton.box->layout();
  }

  QBoxLayout *lay;
  if( mButtonOrientation == Qt::Horizontal )
  {
    lay = new QBoxLayout( d->mButton.box, QBoxLayout::LeftToRight, 0,
			  spacingHint());
  }
  else
  {
    lay = new QBoxLayout( d->mButton.box, QBoxLayout::TopToBottom, 0,
			  spacingHint());
  }

  int numButton = 0;
  QPushButton *prevButton = 0;
  QPushButton *newButton;

  for( int i=0; i<layoutMax; ++i )
  {
    if(((ButtonCode) layout[i]) == Stretch) // Unconditional Stretch
    {
      lay->addStretch(1);
      continue;
    }
    else if (layout[i] & Filler) // Conditional space
    {
      if (d->mButton.mask & layout[i])
      {
        newButton = actionButton( (ButtonCode) (layout[i] & ~(Stretch | Filler)));
        if (newButton)
           lay->addSpacing(newButton->sizeHint().width());
      }
      continue;
    }
    else if( d->mButton.mask & Help & layout[i] )
    {
      newButton = actionButton( Help );
      lay->addWidget( newButton ); ++numButton;
    }
    else if( d->mButton.mask & Default & layout[i] )
    {
      newButton = actionButton( Default );
      lay->addWidget( newButton ); ++numButton;
    }
    else if( d->mButton.mask & User3 & layout[i] )
    {
      newButton = actionButton( User3 );
      lay->addWidget( newButton ); ++numButton;
    }
    else if( d->mButton.mask & User2 & layout[i] )
    {
      newButton = actionButton( User2 );
      lay->addWidget( newButton ); ++numButton;
    }
    else if( d->mButton.mask & User1 & layout[i] )
    {
      newButton = actionButton( User1 );
      lay->addWidget( newButton ); ++numButton;
    }
    else if( d->mButton.mask & Ok & layout[i] )
    {
      newButton = actionButton( Ok );
      lay->addWidget( newButton ); ++numButton;
    }
    else if( d->mButton.mask & Apply & layout[i] )
    {
      newButton = actionButton( Apply );
      lay->addWidget( newButton ); ++numButton;
    }
    else if( d->mButton.mask & Try & layout[i] )
    {
      newButton = actionButton( Try );
      lay->addWidget( newButton ); ++numButton;
    }
    else if( d->mButton.mask & Cancel & layout[i] )
    {
      newButton = actionButton( Cancel );
      lay->addWidget( newButton ); ++numButton;
    }
    else if( d->mButton.mask & Close & layout[i] )
    {
      newButton = actionButton( Close );
      lay->addWidget( newButton ); ++numButton;
    }
    else if( d->mButton.mask & Details & layout[i] )
    {
      newButton = actionButton( Details );
      lay->addWidget( newButton ); ++numButton;
    }
    else
    {
      continue;
    }

    // Add conditional stretch (Only added if a button was added)
    if(layout[i] & Stretch)
    {
      lay->addStretch(1);
    }

    if( prevButton )
    {
      setTabOrder( prevButton, newButton );
    }
    prevButton = newButton;
  }

  d->mButton.resize( false, 0, spacingHint(), mButtonOrientation );
}


KPushButton *KDialogBase::actionButton( ButtonCode id )
{
  return d->mButton.button(id);
}


void KDialogBase::enableButton( ButtonCode id, bool state )
{
  QPushButton *pb = actionButton( id );
  if( pb )
  {
    pb->setEnabled( state );
  }
}


void KDialogBase::enableButtonOK( bool state )
{
  enableButton( Ok, state );
}


void KDialogBase::enableButtonApply( bool state )
{
  enableButton( Apply, state );
}


void KDialogBase::enableButtonCancel( bool state )
{
  enableButton( Cancel, state );
}


void KDialogBase::showButton( ButtonCode id, bool state )
{
  QPushButton *pb = actionButton( id );
  if( pb )
  {
    state ? pb->show() : pb->hide();
  }
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

void KDialogBase::setButtonGuiItem( ButtonCode id, const KGuiItem &item )
{
  KPushButton *pb = static_cast<KPushButton *>( actionButton( id ) );
  if( !pb )
    return;

  pb->setGuiItem( item );
  d->mButton.resize( false, 0, spacingHint(), mButtonOrientation );
}


void KDialogBase::setButtonText( ButtonCode id, const QString &text )
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
    d->mButton.resize( false, 0, spacingHint(), mButtonOrientation );
  }
}


void KDialogBase::setButtonTip( ButtonCode id, const QString &text )
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


void KDialogBase::setButtonWhatsThis( ButtonCode id, const QString &text )
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


void KDialogBase::setButtonFocus( QPushButton *p,bool isDefault, bool isFocus )
{
  p->setDefault( isDefault );
  if( isFocus )
      p->setFocus();
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


void KDialogBase::slotHelp()
{
  emit helpClicked();
  KToolInvocation::invokeHelp( mAnchor, mHelpApp );
}


void KDialogBase::slotDefault()
{
  emit defaultClicked();
}

void KDialogBase::slotDetails()
{
  setDetails(!d->bDetails);
}

void KDialogBase::setDetailsWidget(QWidget *detailsWidget)
{
  delete d->detailsWidget;
  d->detailsWidget = detailsWidget;
  if (d->detailsWidget->parentWidget() != this)
     d->detailsWidget->reparent(this, QPoint(0,0));
  d->detailsWidget->hide();
  if( mIsActivated )
  {
    setupLayout();
  }
  if (!d->bSettingDetails)
    setDetails(d->bDetails);
}

void KDialogBase::setDetails(bool showDetails)
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

void KDialogBase::slotOk()
{
  emit okClicked();
  accept();
}


void KDialogBase::slotApply()
{
  emit applyClicked();
}


void KDialogBase::slotTry()
{
  emit tryClicked();
}


void KDialogBase::slotUser3()
{
  emit user3Clicked();
}


void KDialogBase::slotUser2()
{
  emit user2Clicked();
}


void KDialogBase::slotUser1()
{
  emit user1Clicked();
}


void KDialogBase::slotYes()
{
  emit yesClicked();
  done( Yes );
}


void KDialogBase::slotNo()
{
  emit noClicked();
  done( No );
}


void KDialogBase::slotCancel()
{
  emit cancelClicked();
  done( mMessageBoxMode ? (int)Cancel : (int)Rejected );
}


void KDialogBase::slotClose()
{
  emit closeClicked();
  reject();
}


void KDialogBase::helpClickedSlot( const QString & )
{
  slotHelp();
}


void KDialogBase::applyPressed()
{
  emit apply();
}


void KDialogBase::enableLinkedHelp( bool state )
{
  if( state )
  {
    if( mUrlHelp )
    {
      return;
    }

    mUrlHelp = new KURLLabel( this );
    mUrlHelp->setText( helpLinkText() );
    mUrlHelp->setFloat(true);
    mUrlHelp->setUnderline(true);
    if( mShowTile && mTile->get() )
    {
      mUrlHelp->setBackgroundPixmap(*mTile->get());
    }
    mUrlHelp->setMinimumHeight( fontMetrics().height() + marginHint() );
    connect(mUrlHelp,SIGNAL(leftClickedURL(const QString &)),
	    SLOT(helpClickedSlot(const QString &)));
    mUrlHelp->show();
  }
  else
  {
    if( !mUrlHelp )
    {
      return;
    }
    delete mUrlHelp; mUrlHelp = 0;
  }

  if( mIsActivated )
  {
    setupLayout();
  }
}


void KDialogBase::setHelp( const QString &anchor, const QString &appname )
{
  mAnchor  = anchor;
  mHelpApp = appname;
}


void KDialogBase::setHelpLinkText( const QString &text )
{
  mHelpLinkText = text;
  if( mUrlHelp )
  {
    mUrlHelp->setText( helpLinkText() );
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
  if( mJanus || mMainWidget )
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
  if( mJanus || mMainWidget )
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
  if( mJanus || mMainWidget )
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
  if( mJanus || mMainWidget )
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
  else if( mMainWidget )
  {
    kdDebug() << "makeMainWidget: Main widget already defined" << endl;
  }
}


void KDialogBase::setMainWidget( QWidget *widget )
{
  if( mJanus )
  {
    if( mJanus->setSwallowedWidget(widget) )
    {
      mMainWidget = widget;
    }
  }
  else
  {
    mMainWidget = widget;
    if( mIsActivated )
    {
      setupLayout();
    }
  }
  if( mMainWidget != NULL )
  {
    /* There is no more QFocusData in Qt4, so i used QWidget::nextInFocusChain()
       instead - mattr */
    QWidget* prev = nextInFocusChain();
    foreach ( KDialogBaseButton* button, d->mButton.list )
    {
      if( prev != button )
	setTabOrder( prev, button );
      prev = button;
    }
  }
}


QWidget *KDialogBase::mainWidget()
{
  return mMainWidget;
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

QSize KDialogBase::calculateSize(int w, int h) const
{
  int ulx, uly, lrx, lry;
  ulx = marginHint();
  uly = marginHint();
  if( mUrlHelp  )
  {
    uly += mUrlHelp->minimumSize().height();
  }

  lrx = marginHint();
  lry = d->mButton.box ? d->mButton.box->minimumSize().height() : 0;
  if( mActionSep )
  {
    lry += mActionSep->minimumSize().height() + marginHint();
  }
  return QSize(ulx+w+lrx,uly+h+lry);
}


QString KDialogBase::helpLinkText() const
{
  return ( mHelpLinkText.isNull() ? i18n("Get help...") : mHelpLinkText );
}


void KDialogBase::updateGeometry()
{
  if( mTopLayout )
  {
    mTopLayout->setMargin( marginHint() );
    mTopLayout->setSpacing(spacingHint() );
  }
}



void KDialogBase::keyPressEvent( QKeyEvent *e )
{
  //
  // Reimplemented from KDialog to remove the default behavior
  // and to add F1 (help) sensitivity and some animation.
  //
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
      QPushButton *pb = actionButton( mEscapeButton );
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

  //
  // Do the default action instead. Note KDialog::keyPressEvent is bypassed
  //
  QDialog::keyPressEvent(e);
}



void KDialogBase::hideEvent( QHideEvent *ev )
{
  emit hidden();
  if (!ev->spontaneous())
  {
     emit finished();
  }
}



void KDialogBase::closeEvent( QCloseEvent *e )
{
    QPushButton *pb = actionButton( mEscapeButton );
    if( pb && isShown() ) {
	pb->animateClick();
    } else {
	QDialog::closeEvent( e );
    }
}

void KDialogBase::cancel()
{
    switch ( mEscapeButton ) {
    case Ok:
        slotOk();
        break;
    case User1: // == No
        if ( mMessageBoxMode )
            slotNo();
        else
            slotUser1();
        break;
    case User2: // == Yes
        if ( mMessageBoxMode )
            slotYes();
        else
            slotUser2();
        break;
    case User3:
        slotUser3();
        break;
    case Close:
        slotClose();
        break;
    case Cancel:
    default:
	slotCancel();
    }
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
    if( d->mButton.box )
    {
      d->mButton.box->setBackgroundPixmap(nullPixmap);
      d->mButton.box->setBackgroundMode(Qt::PaletteBackground);
    }
    setBackgroundMode(Qt::PaletteBackground);
  }
  else
  {
    const QPixmap *pix = mTile->get();
    setBackgroundPixmap(*pix);
    if( d->mButton.box )
    {
      d->mButton.box->setBackgroundPixmap(*pix);
    }
    showTile( mShowTile );
  }
}


void KDialogBase::showTile( bool state )
{
  mShowTile = state;
  if( !mShowTile || !mTile || !mTile->get() )
  {
    setBackgroundMode(Qt::PaletteBackground);
    if( d->mButton.box )
    {
      d->mButton.box->setBackgroundMode(Qt::PaletteBackground);
    }
    if( mUrlHelp )
    {
      mUrlHelp->setBackgroundMode(Qt::PaletteBackground);
    }
  }
  else
  {
    const QPixmap *pix = mTile->get();
    setBackgroundPixmap(*pix);
    if( d->mButton.box )
    {
      d->mButton.box->setBackgroundPixmap(*pix);
    }
    if( mUrlHelp )
    {
      mUrlHelp->setBackgroundPixmap(*pix);
    }
  }
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
   w = cg.readNumEntry( QString::fromLatin1("Width %1").arg( desk.width()), w );
   h = cg.readNumEntry( QString::fromLatin1("Height %1").arg( desk.height()), h );

   return QSize( w, h );
}


void KDialogBase::saveDialogSize( const QString& groupName, bool global )
{
   saveDialogSize( *KGlobal::config(), groupName, global );
}


void KDialogBase::saveDialogSize( KConfig& config, const QString& groupName,
				      bool global ) const
{
   int scnum = QApplication::desktop()->screenNumber(parentWidget());
   QRect desk = QApplication::desktop()->screenGeometry(scnum);

   KConfigGroup cg(&config, groupName);
   QSize sizeToSave = size();

   cg.writeEntry( QString::fromLatin1("Width %1").arg( desk.width()),
		  QString::number( sizeToSave.width()), true, global);
   cg.writeEntry( QString::fromLatin1("Height %1").arg( desk.height()),
		  QString::number( sizeToSave.height()), true, global);
}


KDialogBaseButton::KDialogBaseButton( const KGuiItem &item, int key,
				      QWidget *parent )
  : KPushButton( item, parent ), mKey(key)
{
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

#include "kdialogbase.moc"
