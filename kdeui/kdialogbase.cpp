/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999 Mirko Sucker (mirko@kde.org) and 
 *  Espen Sand (espensa@online.no)
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

#include <iostream.h>
#include <stdlib.h>

#include <qlist.h> 
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kdialogbase.h>
#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kseparator.h>
#include <kurllabel.h>

template QList<QPushButton>;

KDialogBaseTile *KDialogBase::mTile = 0;


KDialogBase::KDialogBase( QWidget *parent, const char *name, bool modal,
			  const QString &caption, int buttonMask, 
			  ButtonCode defaultButton, bool separator, 
			  const QString &user1, const QString &user2,
			  const QString &user3 )
  :KDialog( parent, name, modal, WStyle_Customize|WStyle_DialogBorder),
   mMainWidget(0), mJanus(0), mActionSep(0), mIsActivated(false),
   mShowTile(false), mMessageBoxMode(false), mResizeMode(ResizeMinimum)
{
  setCaption( caption );

  makeRelay();
  makeUrlBox();
  connect( this, SIGNAL(layoutHintChanged()), this, SLOT(updateGeometry()) );

  enableButtonSeparator( separator );
  makeButtonBox( buttonMask, defaultButton, user1, user2, user3 );
}


KDialogBase::KDialogBase( int dialogFace, const QString &caption, 
			  int buttonMask, ButtonCode defaultButton, 
			  QWidget *parent, const char *name, bool modal, 
			  bool separator, const QString &user1, 
			  const QString &user2, const QString &user3 )
  :KDialog( parent, name, modal, WStyle_Customize|WStyle_DialogBorder ),
   mMainWidget(0), mJanus(0), mActionSep(0), mIsActivated(false),
   mShowTile(false), mMessageBoxMode(false), mResizeMode(ResizeMinimum)
{
  setCaption( caption );

  makeRelay();
  makeUrlBox();
  connect( this, SIGNAL(layoutHintChanged()), this, SLOT(updateGeometry()) );

  mJanus = new KJanusWidget( this, "janus", dialogFace );
  if( mJanus == 0 || mJanus->isValid() == false ) { return; }

  enableButtonSeparator( separator );
  makeButtonBox( buttonMask, defaultButton, user1, user2, user3 );
}


KDialogBase::KDialogBase( const QString &caption, int buttonMask, 
			  ButtonCode defaultButton, ButtonCode escapeButton, 
			  QWidget *parent, const char *name, bool modal, 
			  bool separator, const QString &yes,
			  const QString &no, const QString &cancel )
  :KDialog( parent, name, modal, WStyle_Customize|WStyle_DialogBorder ),
   mMainWidget(0), mJanus(0), mActionSep(0), mIsActivated(false),
   mShowTile(false), mMessageBoxMode(true), mEscapeButton(escapeButton),
   mResizeMode(ResizeFixed)
{
  setCaption( caption );

  makeRelay();
  makeUrlBox();
  connect( this, SIGNAL(layoutHintChanged()), this, SLOT(updateGeometry()) );

  enableButtonSeparator( separator );

  buttonMask &= Yes|No|Cancel;

  makeButtonBox( buttonMask, defaultButton, no, yes, QString::null );
  setButtonCancelText( cancel );
}



KDialogBase::~KDialogBase( void )
{
}



void KDialogBase::makeRelay( void )
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
    connect( kapp, SIGNAL(aboutToQuit()), mTile, SLOT(cleanup()) );
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
  }
  else
  {
    if( mActionSep == 0 )
    {
      return;
    }
    delete mActionSep; mActionSep = 0;
  }

  if( isVisible() == false )
  {
    return;
  }

  updateGeometry();
}



QFrame *KDialogBase::plainPage( void )
{
  return( mJanus == 0 ? 0 : mJanus->plainPage() );
}


void KDialogBase::show( void )
{
  activateCore();

  if( isVisible() == true )
  {
    raise();
  }
  else
  {
    QDialog::show();

    //
    // 1999-09-09-ES
    // Sometimes it seems that the dialog needs to be kicked into orbit. 
    // This is either a Qt error or something I don't understand because 
    // the dialog is not redrawn until an event occurs (mouse movement, 
    // timer etc).
    //
    kapp->processOneEvent();
  }
}


void KDialogBase::show( QWidget *centerParent )
{
  activateCore();

  if( centerParent != 0 )
  {
    QPoint point = centerParent->mapToGlobal( QPoint(0,0) );
    QRect pos    = centerParent->geometry();
    setGeometry( point.x() + pos.width()/2  - width()/2,
		 point.y() + pos.height()/2 - height()/2, 
		 width(), height() );
  }
  
  if( isVisible() == true )
  {
    raise();
  }
  else
  {
    QDialog::show();
    kapp->processOneEvent(); // See explanation above
  }
}


void KDialogBase::activateCore( void )
{
  if( mIsActivated == true )
  {
    return;
  }

  mIsActivated = true;
  initializeGeometry();

  if( mResizeMode != ResizeFixed )
  {
    resize( minimumSize() + mInitialSizeStep.expandedTo( QSize(0,0) ) );
  }

  if( mJanus != 0 )
  {
    mJanus->showPage(0);
    mJanus->setFocus();
  }
  else if( mMainWidget != 0 )
  {
    // To be decided
  }
}

void KDialogBase::makeUrlBox( void )
{
  QWidget *box = new QWidget( this );
  QHBoxLayout *hbox = new QHBoxLayout( box );
  hbox->addSpacing( marginHint() );
  mUrlHelp = new KURLLabel( box, "url" );
  mUrlHelp->setText(i18n("Get help..."));
  connect(mUrlHelp,SIGNAL(leftClickedURL(const QString &)),
	  SLOT(helpClickedSlot(const QString &)));
  hbox->addWidget( mUrlHelp, 10, AlignRight );
  hbox->addSpacing( marginHint() );
  box->hide();
}


void KDialogBase::setUrlBoxGeometry( void )
{
  QWidget *box = mUrlHelp->parentWidget();
  box->setMinimumHeight( fontMetrics().height() + marginHint() );
  mUrlHelp->setMinimumHeight( fontMetrics().height() + marginHint() );
  mUrlHelp->setAlignment( AlignBottom );
}



void KDialogBase::makeButtonBox( int buttonMask, ButtonCode defaultButton,
				 const QString &user1, const QString &user2,
				 const QString &user3 )
{
  if( buttonMask & Cancel ) { buttonMask &= ~Close; }
  if( buttonMask & Apply ) { buttonMask &= ~Try; }

  if( mMessageBoxMode == false )
  {
    mEscapeButton = (buttonMask&Cancel) ? Cancel : Close;
  }

  mButton.box = new QWidget( this );

  mButton.mask = buttonMask;
  if( mButton.mask & Help )
  {
    QPushButton *pb = mButton.append( Help, i18n("&Help") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotHelp()) );
  }
  if( mButton.mask & Default )
  {
    QPushButton *pb = mButton.append( Default, i18n("&Default") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotDefault()) );
  }
  if( mButton.mask & User3 )
  {
    QPushButton *pb = mButton.append( User3, user3 );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotUser3()) );
  }
  if( mButton.mask & User2 )
  {
    QPushButton *pb = mButton.append( User2, user2 );
    if( mMessageBoxMode == true )
    { 
      connect( pb, SIGNAL(clicked()), this, SLOT(slotYes()) );
    }
    else
    {
      connect( pb, SIGNAL(clicked()), this, SLOT(slotUser2()) );
    }
  }
  if( mButton.mask & User1 )
  {
    QPushButton *pb = mButton.append( User1, user1 );
    if( mMessageBoxMode == true )
    { 
      connect( pb, SIGNAL(clicked()), this, SLOT(slotNo()) );
    }
    else
    {
      connect( pb, SIGNAL(clicked()), this, SLOT(slotUser1()) );
    }
  }
  if( mButton.mask & Ok )
  {
    QPushButton *pb = mButton.append( Ok, i18n("&OK") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotOk()) );
  }
  if( mButton.mask & Apply )
  {
    QPushButton *pb = mButton.append( Apply, i18n("&Apply") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotApply()) );
    connect( pb, SIGNAL(clicked()), this, SLOT(applyPressed()) );
  }
  if( mButton.mask & Try )
  {
    QPushButton *pb = mButton.append( Try, i18n("&Try") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotTry()) );
  }
  if( mButton.mask & Cancel )
  {
    QPushButton *pb = mButton.append( Cancel, i18n("&Cancel") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotCancel()) );
  }
  if( mButton.mask & Close )
  {
    QPushButton *pb = mButton.append( Close, i18n("&Close") );
    connect( pb, SIGNAL(clicked()), this, SLOT(slotClose()) );
  }
  
  QPushButton *pb = getButton( defaultButton );
  if( pb != 0 )
  {
    setButtonFocus( pb, true, true );
  }

  setButtonStyle( ActionStyle0 );
}





void KDialogBase::setButtonStyle( int style )
{
  if( style < 0 || style > ActionStyleMAX ) { style = ActionStyle0; }
  mButton.style = style;

  static int layoutRule[5][8] =
  {
    {Help,Default|Stretch,User3,User2,User1,Ok,Apply|Try,Cancel|Close},
    {Help,Default|Stretch,User3,User2,User1,Cancel|Close,Apply|Try,Ok},
    {Help,Default|Stretch,User3,User2,User1,Apply|Try,Cancel|Close,Ok},
    {Ok,Apply|Try,Cancel|Close,User3,User2,User1|Stretch,Default,Help},
    {Ok,Cancel|Close,Apply|Try,User3,User2,User1|Stretch,Default,Help},
  };
  const int *layout = layoutRule[ mButton.style ];


  if( mButton.box->layout() )
  {
    delete mButton.box->layout();
  }
  QHBoxLayout *hbox = new QHBoxLayout( mButton.box, 0, spacingHint() );
  hbox->addSpacing( marginHint() ); // always


  if( mMessageBoxMode == true )
  {
    hbox->addStretch(1);
  }

  int numButton = 0;
  for( uint i=0; i<8; i++ )
  {
    if( i>0 && (layout[i-1]&Stretch) && mMessageBoxMode == false )
    {
      hbox->addStretch(1);
    }

    if( mButton.mask & Help & layout[i] )
    {
      hbox->addWidget( getButton( Help ) ); numButton++;
    }
    else if( mButton.mask & Default & layout[i] )
    {
      hbox->addWidget( getButton( Default ) ); numButton++;
    }
    else if( mButton.mask & User3 & layout[i] )
    {
      hbox->addWidget( getButton( User3 ) ); numButton++;
    }
    else if( mButton.mask & User2 & layout[i] )
    {
      hbox->addWidget( getButton( User2 ) ); numButton++;
    }
    else if( mButton.mask & User1 & layout[i] )
    {
      hbox->addWidget( getButton( User1 ) ); numButton++;
    }
    else if( mButton.mask & Ok & layout[i] )
    {
      hbox->addWidget( getButton( Ok ) ); numButton++;
    }
    else if( mButton.mask & Apply & layout[i] )
    {
      hbox->addWidget( getButton( Apply ) ); numButton++;
    }
    else if( mButton.mask & Try & layout[i] )
    {
      hbox->addWidget( getButton( Try ) ); numButton++;
    } 
    else if( mButton.mask & Cancel & layout[i] )
    {
      hbox->addWidget( getButton( Cancel ) ); numButton++;
    }
    else if( mButton.mask & Close & layout[i] )
    {
      hbox->addWidget( getButton( Close ) ); numButton++;
    }
    else
    {
      continue;
    }

    if( mMessageBoxMode == true )
    {
      hbox->addStretch(1);
    }

  }

  hbox->addSpacing( marginHint() ); // always
  mButton.resize( true, marginHint(), spacingHint() );
}


QPushButton *KDialogBase::getButton( ButtonCode id )
{
  return( mButton.button(id) );
}


void KDialogBase::enableButton( ButtonCode id, bool state )
{
  QPushButton *pb = getButton( id );
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
  QPushButton *pb = getButton( id );
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
  QPushButton *pb = getButton( Ok );
  if( pb == 0 )
  {
    return;
  }

  const QString whatsThis = i18n( ""
    "If you press the <b>OK</b> button, all changes\n"
    "you made will be used to proceed.");

  pb->setText( text == "" ? i18n("&OK") : text );
  QToolTip::add( pb, tooltip=="" ? i18n("Accept settings") : tooltip );
  QWhatsThis::add( pb, quickhelp == "" ? whatsThis : quickhelp );
}



void KDialogBase::setButtonApplyText( const QString &text, 
				      const QString &tooltip,
				      const QString &quickhelp )
{
  QPushButton *pb = getButton( Apply );
  if( pb == 0 )
  {
    return;
  }

  const QString whatsThis = i18n( ""
    "When clicking <b>Apply</b>, the settings will be\n"
    "handed over to the program, but the dialog\n"
    "will not be closed."
    "Use this to try different settings. ");

  pb->setText( text == "" ? i18n("&Apply") : text );
  QToolTip::add( pb, tooltip == "" ? i18n("Apply settings") : tooltip );
  QWhatsThis::add( pb, quickhelp == "" ? whatsThis : quickhelp );
}


void KDialogBase::setButtonCancelText( const QString& text, 
				       const QString& tooltip, 
				       const QString& quickhelp )
{
  QPushButton *pb = getButton( Cancel );
  if( pb == 0 )
  {
    return;
  }

  const QString whatsThis = i18n( ""
    "If you press the <b>Cancel</b> button, all changes\n"
    "you made will be abandoned and the dialog\n"
    "will be closed.\n"
    "The program will be in the state before\n"
    "opening the dialog.");

  pb->setText( text == "" ? i18n("&Cancel") : text );
  QToolTip::add( pb, tooltip == "" ? i18n("Cancel settings") : tooltip);
  QWhatsThis::add( pb, quickhelp == "" ? whatsThis : quickhelp );
}


void KDialogBase::setButtonText( ButtonCode id, const QString &text )
{
  QPushButton *pb = getButton( id );
  if( pb != 0 )
  {
    pb->setText( text );
  }
}


void KDialogBase::setButtonTip( ButtonCode id, const QString &text )
{
  QPushButton *pb = getButton( id );
  if( pb != 0 )
  {
    QToolTip::add( pb, text );
  }
}


void KDialogBase::setButtonWhatsThis( ButtonCode id, const QString &text )
{
  QPushButton *pb = getButton( id );
  if( pb != 0 )
  {
    QWhatsThis::add( pb, text );
  }
}


void KDialogBase::setButtonFocus( QPushButton *p,bool isDefault, bool isFocus )
{ 
  p->setDefault( isDefault );
  isFocus ? p->setFocus() : p->clearFocus();
}


void KDialogBase::slotHelp( void ) 
{
  emit helpClicked();
  if( mHelpPath.isEmpty() == true )
  {
    kapp->beep();
  } 
  else 
  {
    kapp->invokeHTMLHelp( mHelpPath, mHelpTopic );
  }
}


void KDialogBase::slotDefault( void ) 
{
  emit defaultClicked();
}


void KDialogBase::slotOk( void ) 
{
  emit okClicked();
  accept();
}


void KDialogBase::slotApply( void ) 
{
  emit applyClicked();
}


void KDialogBase::slotTry( void ) 
{
  emit tryClicked();
}


void KDialogBase::slotUser3( void ) 
{
  emit user3Clicked();
}


void KDialogBase::slotUser2( void ) 
{
  emit user2Clicked();
}


void KDialogBase::slotUser1( void )
{
  emit user1Clicked();
}


void KDialogBase::slotYes( void ) 
{
  emit yesClicked();
  done( Yes );
}


void KDialogBase::slotNo( void )
{
  emit noClicked();
  done( No );
}


void KDialogBase::slotCancel( void ) 
{
  emit cancelClicked();
  done( mMessageBoxMode == true ? Cancel : Rejected );
}


void KDialogBase::slotClose( void ) 
{
  emit closeClicked();
  reject();
}


void KDialogBase::helpClickedSlot( const QString & )
{
  slotHelp(); 
}


void KDialogBase::applyPressed( void )
{
  emit apply();
}


void KDialogBase::enableLinkedHelp( bool state )
{
  /*
  if( mButton.mask & Help )
  {
    state = false; // Do not show Help button and link at the same time.
  }
  */

  QWidget *box = mUrlHelp->parentWidget();
  if( box->isVisible() == state )
  {
    return;
  }

  QSize s;
  if( state == false )
  {
    box->hide();
    s = size() - QSize(0,box->minimumSize().height());
    setMinimumSize( minimumSize() - QSize(0,box->minimumSize().height()) );
  }
  else
  {
    setUrlBoxGeometry();
    box->show();
    s = size() + QSize(0,box->minimumSize().height());
    setMinimumSize( minimumSize() + QSize(0,box->minimumSize().height()) );
  }

  if( mIsActivated == true )
  {
    resize(s);
  }
}


void KDialogBase::setHelp( const QString &path, const QString &topic,
			   const QString &text )
{
  mHelpPath  = path;
  mHelpTopic = topic;
  mUrlHelp->setText( text=="" ? i18n("Get help...") : text );
  //enableLinkedHelp( path.isEmpty() == true ? false : true );
}


QFrame *KDialogBase::addPage( const QString &itemName, const QString &header )
{
  return( mJanus == 0 ? 0 : mJanus->addPage( itemName, header ) );
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
    initializeGeometry();
  }
}


QWidget *KDialogBase::getMainWidget( void ) 
{
  return( mMainWidget );
}


bool KDialogBase::showPage( int index )
{
  return( mJanus == 0 ? false : mJanus->showPage(index) );
}


int KDialogBase::activePageIndex( void ) const
{
  return( mJanus == 0 ? -1 : mJanus->activePageIndex() );
}


QRect KDialogBase::getContentsRect( void )
{
  QRect r;
  r.setLeft( marginHint() );
  r.setTop( marginHint() + mUrlHelp->isVisible() ? mUrlHelp->height() : 0 );
  r.setRight( width() - marginHint() );
  int h = (mActionSep==0?0:mActionSep->minimumSize().height()+marginHint());
  r.setBottom( height() - mButton.box->minimumSize().height() - h );
  return(r);
}



void KDialogBase::getBorderWidths(int& ulx, int& uly, int& lrx, int& lry) const
{
  ulx = marginHint();
  uly = marginHint();
  if( mUrlHelp->isVisible() == true )
  {
    uly += mUrlHelp->parentWidget()->minimumSize().height();
  }

  lrx = marginHint();
  lry = mButton.box->minimumSize().height();
  if( mActionSep != 0 ) 
  { 
    lry += mActionSep->minimumSize().height() + marginHint();
  }
}


QSize KDialogBase::calculateSize(int w, int h)
{
  int ulx, uly, lrx, lry;
  getBorderWidths(ulx, uly, lrx, lry);
  return( QSize(ulx+w+lrx,uly+h+lry) );
}


void KDialogBase::setResizeMode( int mode )
{
  mResizeMode = mode;
  initializeGeometry();
}


void KDialogBase::setInitialSizeStep( const QSize &initialSizeStep )
{
  mInitialSizeStep = initialSizeStep;
}


void KDialogBase::updateSize( void )
{
  if( mResizeMode == ResizeFree )
  {
    cout << "KDialogBase: FreeResize not yet implemented" << endl;
    resize( minimumSize() );
  }
  else if( mResizeMode == ResizeFixed )
  {
    setFixedSize( minimumSize() );
    resize( minimumSize() );
  }
  else
  {
    resize( minimumSize() );
  }
}


void KDialogBase::updateGeometry( void )
{
  setUrlBoxGeometry();
  setButtonStyle( mButton.style );
  initializeGeometry();
}


void KDialogBase::initializeGeometry( void )
{
  int m = marginHint();
 
  QWidget *box = mUrlHelp->parentWidget();
  QSize s1 = box->isVisible() == true ? box->minimumSize() : QSize(0,0);

  //
  // Get the minimum size of the janus widget or the custom main widget
  //
  QSize s2;
  if( mJanus != 0 )
  {
    s2 = mJanus->minimumSizeHint();
  }
  else if( mMainWidget != 0 )
  {
    s2 = mMainWidget->minimumSize() + QSize(0,1); // Required for QTextView
    if( s2.isEmpty() == true )
    {
      s2 = mMainWidget->minimumSizeHint();
      if( s2.isEmpty() == true )
      {
	s2 = mMainWidget->sizeHint();
	if( s2.isEmpty() == true )
	{
	  s2 = QSize( 100, 100 ); // Default size
	}
      }
    }
  }
  else
  {
    s2 = QSize( 100, 100 ); // Default size
  }

  QSize s3 = mButton.box->minimumSize();
  if( mActionSep != 0 ) 
  { 
    s3.rheight() += mActionSep->minimumSize().height() + m;
  }

  //
  // Add the margin size. Note that the empty area above the 
  // buttons inside the button box is used as the margin below the mJanus
  // widget. If there is a separator above the box, then there is already
  // added a margin size unit above the separator (see above)
  //
  QSize s4( 2*m, m );

  int w = QMAX( s1.width(), s2.width() );
  w = QMAX( w, s3.width() ) + s4.width();
  int h = s1.height() + s2.height() + s3.height() + s4.height();

  setMinimumSize( w, h );

  if( mIsActivated == true )
  {
    updateSize();
  }
}


void KDialogBase::resizeEvent( QResizeEvent * )
{
  int m = marginHint();

  //
  // The url link box
  //
  QWidget *box = mUrlHelp->parentWidget();
  QSize s1 = (box->isVisible() == false ? QSize(0,0) : box->minimumSize() );
  box->setGeometry( 0, 0, width(), s1.height() );

  //
  // Action button box and (optional) separator. Note that there is an extra
  // marginHint() above the separator.
  //
  QSize s2 = mButton.box->minimumSize();
  mButton.box->setGeometry( 0, height()-s2.height(), width(), s2.height() );
  if( mActionSep != 0 )
  {
    int h = mActionSep->minimumSize().height();
    mActionSep->setGeometry( m, height() - s2.height() - h, width()-2*m, h );
    s2.rheight() += h + m;
  }
  
  //
  // Finally the janus widget or the custom main widget that takes up 
  // the rest of the space
  //
  if( mJanus != 0 )
  {
    mJanus->setGeometry( m, s1.height()+m, width()-2*m,
			 height()-s1.height()-s2.height()-m );
  }
  else if( mMainWidget != 0 )
  {
    mMainWidget->setGeometry( m, s1.height()+m, width()-2*m,
			      height()-s1.height()-s2.height()-m );
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
      QPushButton *pb = getButton( Help );
      if( pb != 0 )
      {
	pb->animateClick();
	e->accept();
	return;
      }
    }
    if( e->key() == Key_Escape )
    {
      QPushButton *pb = getButton( mEscapeButton );
      if( pb != 0 )
      {
	pb->animateClick();
	e->accept();
	return;
      }

    }
  }

  //
  // Do the default action instead. Note KDialog::keyPressEvent is bypassed
  //
  QDialog::keyPressEvent(e);
}



void KDialogBase::hideEvent( QHideEvent * )
{
  emit hidden();
}



void KDialogBase::closeEvent( QCloseEvent *e )
{
  QPushButton *pb = getButton( mEscapeButton );
  if( pb != 0 )
  {
    pb->animateClick();
  }
  else
  {
    QDialog::closeEvent(e);
  }
}


bool KDialogBase::haveBackgroundTile( void )
{
  return( mTile == 0 || mTile->get() == 0 ? false : true );
}


const QPixmap *KDialogBase::getBackgroundTile( void )
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


void KDialogBase::updateBackground( void )
{
  if( mTile == 0 || mTile->get() == 0 )
  {
    QPixmap nullPixmap;
    setBackgroundPixmap(nullPixmap);
    mButton.box->setBackgroundPixmap(nullPixmap);      
    setBackgroundMode(PaletteBackground);
    mButton.box->setBackgroundMode(PaletteBackground);
  }
  else
  {
    const QPixmap *pix = mTile->get();
    setBackgroundPixmap(*pix);
    mButton.box->setBackgroundPixmap(*pix);
    showTile( mShowTile );
  }
}


void KDialogBase::showTile( bool state )
{
  mShowTile = state;
  if( mShowTile == false || mTile == 0 || mTile->get() == 0 )
  {
    setBackgroundMode(PaletteBackground);
    mUrlHelp->parentWidget()->setBackgroundMode(PaletteBackground);
    mButton.box->setBackgroundMode(PaletteBackground);
    mUrlHelp->setTransparentMode( false );
  }
  else
  {
    const QPixmap *pix = mTile->get();
    setBackgroundPixmap(*pix);
    mUrlHelp->parentWidget()->setBackgroundPixmap(*pix);
    mButton.box->setBackgroundPixmap(*pix);
    mUrlHelp->setTransparentMode( true );
  }
}


void KDialogBase::emitBackgroundChanged( void )
{
  emit backgroundChanged();
}





KDialogBaseButton::KDialogBaseButton( const QString &text, int key,
				      QWidget *parent,  const char *name )
  : QPushButton( text, parent, name )
{
  mKey = key;
}




KDialogBaseTile::KDialogBaseTile( QObject *parent, const char *name )
  : QObject( parent, name )
{
  mPixmap = 0;
}


KDialogBaseTile::~KDialogBaseTile( void )
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


const QPixmap *KDialogBaseTile::get( void ) const
{
  return( mPixmap );
}


void KDialogBaseTile::cleanup( void )
{
  delete mPixmap; mPixmap = 0;
}




#include "kdialogbase.moc"







