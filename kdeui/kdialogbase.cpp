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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include "kdialogbase.h"
#include <stdlib.h>

#include <qgrid.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qtimer.h>

#include <kapplication.h>
#include <klocale.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kseparator.h>
#include <kurllabel.h>
#include <kdebug.h>

#include "kdialogbase_priv.h"
#include "kdialogbase_priv.moc"

KDialogBaseTile *KDialogBase::mTile = 0;

int KDialogBaseButton::id()
{
    return( mKey );
}

template class QPtrList<KDialogBaseButton>;

/**
 * @internal
 */
struct SButton : public Qt
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
  QPtrList<KDialogBaseButton> list;
};

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
  :KDialog( parent, name, modal, WStyle_DialogBorder ),
   mTopLayout(0), mMainWidget(0), mUrlHelp(0), mJanus(0), mActionSep(0),
   mIsActivated(false), mShowTile(false), mMessageBoxMode(false),
   mButtonOrientation(Horizontal)
{
  d = new KDialogBasePrivate;
  setCaption( caption );

  makeRelay();
  connect( this, SIGNAL(layoutHintChanged()), this, SLOT(updateGeometry()) );

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
  :KDialog( parent, name, modal, WStyle_DialogBorder ),
   mTopLayout(0), mMainWidget(0), mUrlHelp(0), mJanus(0), mActionSep(0),
   mIsActivated(false), mShowTile(false), mMessageBoxMode(false),
   mButtonOrientation(Horizontal)
{
  d = new KDialogBasePrivate;
  setCaption( caption );

  makeRelay();
  connect( this, SIGNAL(layoutHintChanged()), this, SLOT(updateGeometry()) );

  mJanus = new KJanusWidget( this, "janus", dialogFace );
  connect(mJanus, SIGNAL(aboutToShowPage(QWidget *)),
          this, SIGNAL(aboutToShowPage(QWidget *)));

  if( mJanus == 0 || mJanus->isValid() == false ) { return; }

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
  :KDialog( parent, name, modal, WStyle_DialogBorder ),
   mTopLayout(0), mMainWidget(0), mUrlHelp(0), mJanus(0), mActionSep(0),
   mIsActivated(false), mShowTile(false), mMessageBoxMode(true),
   mButtonOrientation(Horizontal),mEscapeButton(escapeButton)
{
  d = new KDialogBasePrivate;

  setCaption( caption );

  makeRelay();
  connect( this, SIGNAL(layoutHintChanged()), this, SLOT(updateGeometry()) );

  enableButtonSeparator( separator );

  buttonMask &= Details|Yes|No|Cancel;

  makeButtonBox( buttonMask, defaultButton,
                 no.text().isEmpty()  ? KStdGuiItem::no()  : no,
                 yes.text().isEmpty() ? KStdGuiItem::yes() : yes );

  setButtonCancelText( cancel.text().isEmpty() ?
                       KStdGuiItem::cancel().text() : cancel.text() );

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
  return( p );
}

void SButton::resize( bool sameWidth, int margin,
    int spacing, int orientation )
{
  KDialogBaseButton *p;
  int w = 0;
  int t = 0;

  for( p = list.first(); p!=0; p =  list.next() )
  {
    if( p->sizeHint().width() > w ) { w = p->sizeHint().width(); }
  }

  if( orientation == Horizontal )
  {
    for( p = list.first(); p!=0; p =  list.next() )
    {
      QSize s( p->sizeHint() );
      if( sameWidth == true ) { s.setWidth( w ); }
      p->setFixedWidth( s.width() );
      t += s.width() + spacing;
    }

    p = list.first();
    box->setMinimumHeight( margin*2 + ( p==0?0:p->sizeHint().height()));
    box->setMinimumWidth( margin*2 + t - spacing );
  }
  else
  {
    // sameWidth has no effect here
    for( p = list.first(); p!=0; p =  list.next() )
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
  KDialogBaseButton *p;
  for( p = list.first(); p != 0; p = list.next() )
  {
    if( p->id() == key )
    {
      return( p );
    }
  }
  return( 0 );
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
  if( mTopLayout != 0 )
  {
    delete mTopLayout;
  }
  // mTopLayout = new QVBoxLayout( this, marginHint(), spacingHint() );


  if( mButtonOrientation == Horizontal )
  {
    mTopLayout = new QBoxLayout( this, QBoxLayout::TopToBottom,
				 marginHint(), spacingHint() );
  }
  else
  {
    mTopLayout = new QBoxLayout( this, QBoxLayout::LeftToRight,
				 marginHint(), spacingHint() );
  }

  if( mUrlHelp != 0 )
  {
    mTopLayout->addWidget( mUrlHelp, 0, AlignRight );
  }

  if( mJanus != 0 )
  {
    mTopLayout->addWidget( mJanus, 10 );
  }
  else if( mMainWidget != 0 )
  {
    mTopLayout->addWidget( mMainWidget, 10 );
  }

  if ( d->detailsWidget )
  {
    mTopLayout->addWidget( d->detailsWidget );
  }

  if( mActionSep != 0 )
  {
    mTopLayout->addWidget( mActionSep );
  }

  if( d->mButton.box != 0 )
  {
    mTopLayout->addWidget( d->mButton.box );
  }
}



void KDialogBase::setButtonBoxOrientation( int orientation )
{
  if( mButtonOrientation != orientation )
  {
    mButtonOrientation = orientation;
    if( mActionSep != 0 )
    {
      mActionSep->setOrientation( mButtonOrientation == Horizontal ?
				  QFrame::HLine : QFrame::VLine );
    }
    if( mButtonOrientation == Vertical )
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
  if( mTile != 0 )
  {
    connect( mTile, SIGNAL(pixmapChanged()), this, SLOT(updateBackground()) );
    return;
  }

  mTile = new KDialogBaseTile;
  if( mTile != 0 )
  {
    connect( mTile, SIGNAL(pixmapChanged()), this, SLOT(updateBackground()) );
    connect( qApp, SIGNAL(aboutToQuit()), mTile, SLOT(cleanup()) );
  }
}


void KDialogBase::enableButtonSeparator( bool state )
{
  if( state == true )
  {
    if( mActionSep != 0 )
    {
      return;
    }
    mActionSep = new KSeparator( this );
    mActionSep->setFocusPolicy(QWidget::NoFocus);
    mActionSep->setOrientation( mButtonOrientation == Horizontal ?
				QFrame::HLine : QFrame::VLine );
    mActionSep->show();
  }
  else
  {
    if( mActionSep == 0 )
    {
      return;
    }
    delete mActionSep; mActionSep = 0;
  }

  if( mIsActivated == true )
  {
    setupLayout();
  }
}



QFrame *KDialogBase::plainPage()
{
  return( mJanus == 0 ? 0 : mJanus->plainPage() );
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
  int m = marginHint();
  int s = spacingHint();

  QSize s1(0,0);
  QSize s2(0,0);

  //
  // Url help area
  //
  if( mUrlHelp != 0 )
  {
    s2 = mUrlHelp->minimumSize() + QSize( 0, s );
  }
  s1.rwidth()   = QMAX( s1.rwidth(), s2.rwidth() );
  s1.rheight() += s2.rheight();

  //
  // User widget
  //
  if( mJanus != 0 )
  {
    s2 = mJanus->minimumSizeHint() + QSize( 0, s );
  }
  else if( mMainWidget != 0 )
  {
    s2 = mMainWidget->sizeHint() + QSize( 0, s );
    s2 = s2.expandedTo( mMainWidget->minimumSize() );
    s2 = s2.expandedTo( mMainWidget->minimumSizeHint() );
    if( s2.isEmpty() == true )
    {
      s2 = QSize( 100, 100+s );
    }
  }
  else
  {
    s2 = QSize( 100, 100+s );
  }
  s1.rwidth()  = QMAX( s1.rwidth(), s2.rwidth() );
  s1.rheight() += s2.rheight();

  if (d->detailsWidget && d->bDetails)
  {
    s2 = d->detailsWidget->sizeHint() + QSize( 0, s );
    s2 = s2.expandedTo( d->detailsWidget->minimumSize() );
    s2 = s2.expandedTo( d->detailsWidget->minimumSizeHint() );
    s1.rwidth()  = QMAX( s1.rwidth(), s2.rwidth() );
    s1.rheight() += s2.rheight();
  }

  //
  // Button separator
  //
  if( mActionSep != 0 )
  {
    s1.rheight() += mActionSep->minimumSize().height() + s;
  }

  //
  // The button box
  //
  if( d->mButton.box != 0 )
  {
    s2 = d->mButton.box->minimumSize();
    if( mButtonOrientation == Horizontal )
    {
      s1.rwidth()   = QMAX( s1.rwidth(), s2.rwidth() );
      s1.rheight() += s2.rheight();
    }
    else
    {
      s1.rwidth() += s2.rwidth();
      s1.rheight() = QMAX( s1.rheight(), s2.rheight() );
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

  if( mMessageBoxMode == false )
  {
    mEscapeButton = (buttonMask&Cancel) ? Cancel : Close;
  }

  d->mButton.box = new QWidget( this );

  d->mButton.mask = buttonMask;
  if( d->mButton.mask & Help )
  {
    KPushButton *pb = d->mButton.append( Help, KStdGuiItem::help() );

    connect( pb, SIGNAL(clicked()), this, SLOT(slotHelp()) );
  }
  if( d->mButton.mask & Default )
  {
    KPushButton *pb = d->mButton.append( Default, KStdGuiItem::defaults() );

    connect( pb, SIGNAL(clicked()), this, SLOT(slotDefault()) );
  }
  if( d->mButton.mask & Details )
  {
    KPushButton *pb = d->mButton.append( Details, QString::null );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotDetails()) );
    setDetails(false);
  }
  if( d->mButton.mask & User3 )
  {
    KPushButton *pb = d->mButton.append( User3, user3 );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotUser3()) );
  }
  if( d->mButton.mask & User2 )
  {
    KPushButton *pb = d->mButton.append( User2, user2 );
    if( mMessageBoxMode == true )
    {
      connect( pb, SIGNAL(clicked()), this, SLOT(slotYes()) );
    }
    else
    {
      connect( pb, SIGNAL(clicked()), this, SLOT(slotUser2()) );
    }
  }
  if( d->mButton.mask & User1 )
  {
    KPushButton *pb = d->mButton.append( User1, user1 );
    if( mMessageBoxMode == true )
    {
      connect( pb, SIGNAL(clicked()), this, SLOT(slotNo()) );
    }
    else
    {
      connect( pb, SIGNAL(clicked()), this, SLOT(slotUser1()) );
    }
  }
  if( d->mButton.mask & Ok )
  {
    KPushButton *pb = d->mButton.append( Ok, KStdGuiItem::ok() );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotOk()) );
  }
  if( d->mButton.mask & Apply )
  {
    KPushButton *pb = d->mButton.append( Apply, KStdGuiItem::apply() );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotApply()) );
    connect( pb, SIGNAL(clicked()), this, SLOT(applyPressed()) );
  }
  if( d->mButton.mask & Try )
  {
    KPushButton *pb = d->mButton.append( Try,
                          KGuiItem( i18n( "&Try" ), "try" ) );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotTry()) );
  }
  if( d->mButton.mask & Cancel )
  {
    KPushButton *pb = d->mButton.append( Cancel, KStdGuiItem::cancel() );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotCancel()) );
  }
  if( d->mButton.mask & Close )
  {
    KPushButton *pb = d->mButton.append( Close, KStdGuiItem::close() );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotClose()) );
  }

  QPushButton *pb = actionButton( defaultButton );
  if( pb != 0 )
  {
    setButtonFocus( pb, true, true );
  }

  setButtonStyle( ActionStyle0 );
}



void KDialogBase::setButtonStyle( int style )
{
  if( d->mButton.box == 0 )
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
  else if (mButtonOrientation == Horizontal)
  {
    static const int layoutRule[5][9] =
    {
      {Help,Default,Stretch,User3,User2,User1,Ok,Apply|Try,Cancel|Close},
      {Help,Default,Stretch,User3,User2,User1,Cancel|Close,Apply|Try,Ok},
      {Help,Default,Stretch,User3,User2,User1,Apply|Try,Cancel|Close,Ok},
      {Ok,Apply|Try,Cancel|Close,User3,User2,User1,Stretch,Default,Help},
      {Ok,Cancel|Close,Apply|Try,User3,User2,User1,Stretch,Default,Help}
    };
    layoutMax = 9;
    layout = layoutRule[ d->mButton.style ];
  }
  else
  {
    static const int layoutRule[5][9] =
    {
      {Ok,Apply|Try,User1,User2,User3,Stretch,Default,Cancel|Close,Help},
      //{Ok,Apply|Try,Cancel|Close,User1,User2,User3,Stretch, Default,Help},
      {Help,Default,Stretch,User3,User2,User1,Cancel|Close,Apply|Try,Ok},
      {Help,Default,Stretch,User3,User2,User1,Apply|Try,Cancel|Close,Ok},
      {Ok,Apply|Try,Cancel|Close,User3,User2,User1,Stretch,Default,Help},
      {Ok,Cancel|Close,Apply|Try,User3,User2,User1,Stretch,Default,Help}
    };
    layoutMax = 9;
    layout = layoutRule[ d->mButton.style ];
  }

  if( d->mButton.box->layout() )
  {
    delete d->mButton.box->layout();
  }

  QBoxLayout *lay;
  if( mButtonOrientation == Horizontal )
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

  for( int i=0; i<layoutMax; i++ )
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
      lay->addWidget( newButton ); numButton++;
    }
    else if( d->mButton.mask & Default & layout[i] )
    {
      newButton = actionButton( Default );
      lay->addWidget( newButton ); numButton++;
    }
    else if( d->mButton.mask & User3 & layout[i] )
    {
      newButton = actionButton( User3 );
      lay->addWidget( newButton ); numButton++;
    }
    else if( d->mButton.mask & User2 & layout[i] )
    {
      newButton = actionButton( User2 );
      lay->addWidget( newButton ); numButton++;
    }
    else if( d->mButton.mask & User1 & layout[i] )
    {
      newButton = actionButton( User1 );
      lay->addWidget( newButton ); numButton++;
    }
    else if( d->mButton.mask & Ok & layout[i] )
    {
      newButton = actionButton( Ok );
      lay->addWidget( newButton ); numButton++;
    }
    else if( d->mButton.mask & Apply & layout[i] )
    {
      newButton = actionButton( Apply );
      lay->addWidget( newButton ); numButton++;
    }
    else if( d->mButton.mask & Try & layout[i] )
    {
      newButton = actionButton( Try );
      lay->addWidget( newButton ); numButton++;
    }
    else if( d->mButton.mask & Cancel & layout[i] )
    {
      newButton = actionButton( Cancel );
      lay->addWidget( newButton ); numButton++;
    }
    else if( d->mButton.mask & Close & layout[i] )
    {
      newButton = actionButton( Close );
      lay->addWidget( newButton ); numButton++;
    }
    else if( d->mButton.mask & Details & layout[i] )
    {
      newButton = actionButton( Details );
      lay->addWidget( newButton ); numButton++;
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

    if( prevButton != 0 )
    {
      setTabOrder( prevButton, newButton );
    }
    prevButton = newButton;
  }

  d->mButton.resize( false, 0, spacingHint(), mButtonOrientation );
}


QPushButton *KDialogBase::actionButton( ButtonCode id )
{
  return( d->mButton.button(id) );
}


void KDialogBase::enableButton( ButtonCode id, bool state )
{
  QPushButton *pb = actionButton( id );
  if( pb != 0 )
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
  if( pb != 0 )
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


void KDialogBase::setButtonOKText( const QString &text,
				   const QString &tooltip,
				   const QString &quickhelp )
{
  QPushButton *pb = actionButton( Ok );
  if( pb == 0 )
  {
    return;
  }

  const QString whatsThis = i18n( ""
    "If you press the <b>OK</b> button, all changes\n"
    "you made will be used to proceed.");

  pb->setText( text.isEmpty() ? i18n("&OK") : text );
  d->mButton.resize( false, 0, spacingHint(), mButtonOrientation );

  QToolTip::add( pb, tooltip.isEmpty() ? i18n("Accept settings") : tooltip );
  QWhatsThis::add( pb, quickhelp.isEmpty() ? whatsThis : quickhelp );
}



void KDialogBase::setButtonApplyText( const QString &text,
				      const QString &tooltip,
				      const QString &quickhelp )
{
  QPushButton *pb = actionButton( Apply );
  if( pb == 0 )
  {
    return;
  }

  const QString whatsThis = i18n( ""
    "When clicking <b>Apply</b>, the settings will be\n"
    "handed over to the program, but the dialog\n"
    "will not be closed. "
    "Use this to try different settings. ");

  pb->setText( text.isEmpty() ? i18n("&Apply") : text );
  d->mButton.resize( false, 0, spacingHint(), mButtonOrientation );

  QToolTip::add( pb, tooltip.isEmpty() ? i18n("Apply settings") : tooltip );
  QWhatsThis::add( pb, quickhelp.isEmpty() ? whatsThis : quickhelp );
}


void KDialogBase::setButtonCancelText( const QString& text,
				       const QString& tooltip,
				       const QString& quickhelp )
{
  QPushButton *pb = actionButton( Cancel );
  if( pb == 0 )
  {
    return;
  }

  pb->setText( text.isEmpty() ? i18n("&Cancel") : text );
  d->mButton.resize( false, 0, spacingHint(), mButtonOrientation );

  QToolTip::add( pb, tooltip );
  QWhatsThis::add( pb, quickhelp );
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
  if( pb != 0 )
  {
    pb->setText( text );
    d->mButton.resize( false, 0, spacingHint(), mButtonOrientation );
  }
}


void KDialogBase::setButtonTip( ButtonCode id, const QString &text )
{
  QPushButton *pb = actionButton( id );
  if( pb != 0 )
  {
    if (text.isEmpty())
      QToolTip::remove( pb );
    else
      QToolTip::add( pb, text );
  }
}


void KDialogBase::setButtonWhatsThis( ButtonCode id, const QString &text )
{
  QPushButton *pb = actionButton( id );
  if( pb != 0 )
  {
    if (text.isEmpty())
      QWhatsThis::remove( pb );
    else
      QWhatsThis::add( pb, text );
  }
}


void KDialogBase::setButtonFocus( QPushButton *p,bool isDefault, bool isFocus )
{
  p->setDefault( isDefault );
  isFocus ? p->setFocus() : p->clearFocus();
}


void KDialogBase::setTreeListAutoResize( bool state )
{
  if( mJanus != 0 )
  {
    mJanus->setTreeListAutoResize( state );
  }
}

void KDialogBase::setShowIconsInTreeList(bool state)
{
  if( mJanus != 0 )
  {
    mJanus->setShowIconsInTreeList( state );
  }
}

void KDialogBase::setRootIsDecorated( bool state )
{
  if( mJanus != 0 )
  {
    mJanus->setRootIsDecorated( state );
  }
}



void KDialogBase::setIconListAllVisible( bool state )
{
  if( mJanus != 0 )
  {
    mJanus->setIconListAllVisible( state );
  }
}


void KDialogBase::slotHelp()
{
  emit helpClicked();
  if ( kapp ) // may be null when used inside designer
      kapp->invokeHelp( mAnchor, mHelpApp );
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
  if( mIsActivated == true )
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
  done( mMessageBoxMode == true ? (int)Cancel : (int)Rejected );
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
  if( state == true )
  {
    if( mUrlHelp != 0 )
    {
      return;
    }

    mUrlHelp = new KURLLabel( this, "url" );
    mUrlHelp->setText( helpLinkText() );
    mUrlHelp->setFloat(true);
    mUrlHelp->setUnderline(true);
    if( mShowTile == true && mTile->get() != 0 )
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
    if( mUrlHelp == 0 )
    {
      return;
    }
    delete mUrlHelp; mUrlHelp = 0;
  }

  if( mIsActivated == true )
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
  if( mUrlHelp != 0 )
  {
    mUrlHelp->setText( helpLinkText() );
  }
}


QFrame *KDialogBase::addPage( const QString &itemName, const QString &header,
			      const QPixmap &pixmap )
{
  return( mJanus == 0 ? 0 : mJanus->addPage( itemName, header, pixmap ) );
}

QFrame *KDialogBase::addPage( const QStringList &items, const QString &header,
			      const QPixmap &pixmap )
{
  return( mJanus == 0 ? 0 : mJanus->addPage( items, header, pixmap ) );
}


QVBox *KDialogBase::addVBoxPage( const QString &itemName,
				 const QString &header, const QPixmap &pixmap )
{
  return( mJanus == 0 ? 0 : mJanus->addVBoxPage( itemName, header, pixmap) );
}

QVBox *KDialogBase::addVBoxPage( const QStringList &items,
				 const QString &header, const QPixmap &pixmap )
{
  return( mJanus == 0 ? 0 : mJanus->addVBoxPage( items, header, pixmap) );
}


QHBox *KDialogBase::addHBoxPage( const QString &itemName,
				 const QString &header,
				 const QPixmap &pixmap )
{
  return( mJanus == 0 ? 0 : mJanus->addHBoxPage( itemName, header, pixmap ) );
}

QHBox *KDialogBase::addHBoxPage( const QStringList &items,
				 const QString &header,
				 const QPixmap &pixmap )
{
  return( mJanus == 0 ? 0 : mJanus->addHBoxPage( items, header, pixmap ) );
}


QGrid *KDialogBase::addGridPage( int n, Orientation dir,
				 const QString &itemName,
				 const QString &header, const QPixmap &pixmap )
{
  return( mJanus == 0 ? 0 : mJanus->addGridPage( n, dir, itemName, header,
						 pixmap) );
}

QGrid *KDialogBase::addGridPage( int n, Orientation dir,
				 const QStringList &items,
				 const QString &header, const QPixmap &pixmap )
{
  return( mJanus == 0 ? 0 : mJanus->addGridPage( n, dir, items, header,
						 pixmap) );
}

void KDialogBase::setFolderIcon(const QStringList &path, const QPixmap &pixmap)
{
  if (mJanus == 0)
    return;

  mJanus->setFolderIcon(path,pixmap);
}

QFrame *KDialogBase::makeMainWidget()
{
  if( mJanus != 0 || mMainWidget != 0 )
  {
    printMakeMainWidgetError();
    return( 0 );
  }

  QFrame *mainWidget = new QFrame( this );
  setMainWidget( mainWidget );
  return( mainWidget );
}


QVBox *KDialogBase::makeVBoxMainWidget()
{
  if( mJanus != 0 || mMainWidget != 0 )
  {
    printMakeMainWidgetError();
    return( 0 );
  }

  QVBox *mainWidget = new QVBox( this );
  mainWidget->setSpacing( spacingHint() );
  setMainWidget( mainWidget );
  return( mainWidget );
}


QHBox *KDialogBase::makeHBoxMainWidget()
{
  if( mJanus != 0 || mMainWidget != 0 )
  {
    printMakeMainWidgetError();
    return( 0 );
  }

  QHBox *mainWidget = new QHBox( this );
  mainWidget->setSpacing( spacingHint() );
  setMainWidget( mainWidget );
  return( mainWidget );
}


QGrid *KDialogBase::makeGridMainWidget( int n, Orientation dir )
{
  if( mJanus != 0 || mMainWidget != 0 )
  {
    printMakeMainWidgetError();
    return( 0 );
  }

  QGrid *mainWidget = new QGrid( n, dir, this );
  mainWidget->setSpacing( spacingHint() );
  setMainWidget( mainWidget );
  return( mainWidget );
}


void KDialogBase::printMakeMainWidgetError()
{
  if( mJanus != 0 )
  {
    kdDebug() << "makeMainWidget: Illegal mode (wrong constructor)" << endl;
  }
  else if( mMainWidget != 0 )
  {
    kdDebug() << "makeMainWidget: Main widget already defined" << endl;
  }
}


void KDialogBase::setMainWidget( QWidget *widget )
{
  if( mJanus != 0 )
  {
    if( mJanus->setSwallowedWidget(widget) == true )
    {
      mMainWidget = widget;
    }
  }
  else
  {
    mMainWidget = widget;
    if( mIsActivated == true )
    {
      setupLayout();
    }
  }
}


QWidget *KDialogBase::mainWidget()
{
  return( mMainWidget );
}


bool KDialogBase::showPage( int index )
{
  return( mJanus == 0 ? false : mJanus->showPage(index) );
}


int KDialogBase::activePageIndex() const
{
  return( mJanus == 0 ? -1 : mJanus->activePageIndex() );
}


int KDialogBase::pageIndex( QWidget *widget ) const
{
  return( mJanus == 0 ? -1 : mJanus->pageIndex( widget) );
}


// Deprecated
QRect KDialogBase::getContentsRect() const
{
  QRect r;
  r.setLeft( marginHint() );
  r.setTop( marginHint() + (mUrlHelp != 0 ? mUrlHelp->height() : 0) );
  r.setRight( width() - marginHint() );
  int h = (mActionSep==0?0:mActionSep->minimumSize().height()+marginHint());
  if( d->mButton.box != 0 )
  {
    r.setBottom( height() - d->mButton.box->minimumSize().height() - h );
  }
  else
  {
    r.setBottom( height() - h );
  }

  return(r);
}


// Deprecated
void KDialogBase::getBorderWidths(int& ulx, int& uly, int& lrx, int& lry) const
{
  ulx = marginHint();
  uly = marginHint();
  if( mUrlHelp != 0  )
  {
    uly += mUrlHelp->minimumSize().height();
  }

  lrx = marginHint();
  lry = d->mButton.box != 0 ? d->mButton.box->minimumSize().height() : 0;
  if( mActionSep != 0 )
  {
    lry += mActionSep->minimumSize().height() + marginHint();
  }
}


QSize KDialogBase::calculateSize(int w, int h) const
{
  int ulx, uly, lrx, lry;
  getBorderWidths(ulx, uly, lrx, lry);
  return( QSize(ulx+w+lrx,uly+h+lry) );
}


QString KDialogBase::helpLinkText() const
{
  return( mHelpLinkText==QString::null ? i18n("Get help...") : mHelpLinkText );
}


void KDialogBase::updateGeometry()
{
  if( mTopLayout != 0 )
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
    if( e->key() == Key_F1 )
    {
      QPushButton *pb = actionButton( Help );
      if( pb != 0 )
      {
	pb->animateClick();
	e->accept();
	return;
      }
    }
    if( e->key() == Key_Escape )
    {
      QPushButton *pb = actionButton( mEscapeButton );
      if( pb != 0 )
      {
	pb->animateClick();
	e->accept();
	return;
      }

    }
  }
  else if( e->key() == Key_F1 && e->state() == ShiftButton )
  {
    QWhatsThis::enterWhatsThisMode();
    e->accept();
    return;
  }
  
  // accept the dialog when Ctrl-Return is pressed
  else if ( e->state() == ControlButton && 
            qApp->focusWidget() && 
            qApp->focusWidget()->inherits( "QTextEdit" ) &&
            (e->key() == Key_Return || e->key() == Key_Enter) )
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
    if( pb != 0 ) {
	pb->animateClick();
    } else {
	QDialog::closeEvent( e );
    }
}


bool KDialogBase::haveBackgroundTile()
{
  return( mTile == 0 || mTile->get() == 0 ? false : true );
}

// Deprecated. For compatibility only.
const QPixmap *KDialogBase::getBackgroundTile() { return backgroundTile(); }

const QPixmap *KDialogBase::backgroundTile()
{
  return( mTile == 0 ? 0 : mTile->get() );
}


void KDialogBase::setBackgroundTile( const QPixmap *pix )
{
  if( mTile != 0 )
  {
    mTile->set( pix );
  }
}


void KDialogBase::updateBackground()
{
  if( mTile == 0 || mTile->get() == 0 )
  {
    QPixmap nullPixmap;
    setBackgroundPixmap(nullPixmap);
    if( d->mButton.box != 0 )
    {
      d->mButton.box->setBackgroundPixmap(nullPixmap);
      d->mButton.box->setBackgroundMode(PaletteBackground);
    }
    setBackgroundMode(PaletteBackground);
  }
  else
  {
    const QPixmap *pix = mTile->get();
    setBackgroundPixmap(*pix);
    if( d->mButton.box != 0 )
    {
      d->mButton.box->setBackgroundPixmap(*pix);
    }
    showTile( mShowTile );
  }
}


void KDialogBase::showTile( bool state )
{
  mShowTile = state;
  if( mShowTile == false || mTile == 0 || mTile->get() == 0 )
  {
    setBackgroundMode(PaletteBackground);
    if( d->mButton.box != 0 )
    {
      d->mButton.box->setBackgroundMode(PaletteBackground);
    }
    if( mUrlHelp != 0 )
    {
      mUrlHelp->setBackgroundMode(PaletteBackground);
    }
  }
  else
  {
    const QPixmap *pix = mTile->get();
    setBackgroundPixmap(*pix);
    if( d->mButton.box != 0 )
    {
      d->mButton.box->setBackgroundPixmap(*pix);
    }
    if( mUrlHelp != 0 )
    {
      mUrlHelp->setBackgroundPixmap(*pix);
    }
  }
}

QSize KDialogBase::configDialogSize( const QString& groupName ) const
{
   int w, h;
   int scnum = QApplication::desktop()->screenNumber(parentWidget());
   QRect desk = QApplication::desktop()->screenGeometry(scnum);
   w = QMIN( 530, (int) (desk.width() * 0.5)); // maximum default width = 530
   h = (int) (desk.height() * 0.4);

   KConfig *kc = KGlobal::config();

   if( kc )
   {
      KConfigGroupSaver cs(kc, groupName);
      w = kc->readNumEntry( QString::fromLatin1("Width %1").arg( desk.width()), w );
      h = kc->readNumEntry( QString::fromLatin1("Height %1").arg( desk.height()), h );
   }
   return( QSize( w, h ) );
}


void KDialogBase::saveDialogSize( const QString& groupName, bool global )
{
   int scnum = QApplication::desktop()->screenNumber(parentWidget());
   QRect desk = QApplication::desktop()->screenGeometry(scnum);
   KConfig *kc = KGlobal::config();

   if( kc )
   {
      KConfigGroupSaver cs(kc, groupName);
      QSize sizeToSave = size();

      kc->writeEntry( QString::fromLatin1("Width %1").arg( desk.width()),
		      QString::number( sizeToSave.width()), true, global);
      kc->writeEntry( QString::fromLatin1("Height %1").arg( desk.height()),
		      QString::number( sizeToSave.height()), true, global);
   }
}


KDialogBaseButton::KDialogBaseButton( const KGuiItem &item, int key,
				      QWidget *parent,  const char *name )
  : KPushButton( item, parent, name )
{
  mKey = key;
}




KDialogBaseTile::KDialogBaseTile( QObject *parent, const char *name )
  : QObject( parent, name )
{
  mPixmap = 0;
}


KDialogBaseTile::~KDialogBaseTile()
{
  cleanup();
}


void KDialogBaseTile::set( const QPixmap *pix )
{
  if( pix == 0 )
  {
    cleanup();
  }
  else
  {
    if( mPixmap == 0 )
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
  return( mPixmap );
}


void KDialogBaseTile::cleanup()
{
  delete mPixmap; mPixmap = 0;
}

void KDialogBase::virtual_hook( int id, void* data )
{ KDialog::virtual_hook( id, data ); }

#include "kdialogbase.moc"
