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


QPixmap *KDialogBase::mTile  = 0;
KDialogBase *KDialogBase::mRelay = 0;



KDialogBase::KDialogBase( QWidget *parent, const char *name, bool modal,
			  const QString &caption, int buttonMask, 
			  int defaultButton, bool separator, 
			  const QString &user1, const QString &user2,
			  const QString &user3 )
  :KDialog( parent, name, modal, WStyle_Customize|WStyle_DialogBorder),
   mMainWidget(0), mJanus(0), mActionSep(0), mIsActivated(false),
   mShowTile(false), mResizeMode(ResizeMinimum)
{
  setCaption( caption );

  makeRelay();
  makeUrlBox();
  connect( this, SIGNAL(layoutHintChanged()), this, SLOT(updateGeometry()) );

  enableButtonSeparator( separator );
  makeButtonBox( buttonMask, defaultButton, user1, user2, user3 );
}


KDialogBase::KDialogBase( int dialogFace, const QString &caption, 
			  int buttonMask, int defaultButton, QWidget *parent, 
			  const char *name, bool modal, bool separator,
			  const QString &user1, const QString &user2,
			  const QString &user3 )
  :KDialog( parent, name, modal, WStyle_Customize|WStyle_DialogBorder ),
   mMainWidget(0), mJanus(0), mActionSep(0), mIsActivated(false),
   mShowTile(false), mResizeMode(ResizeMinimum)
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


KDialogBase::~KDialogBase( void )
{
}


void KDialogBase::cleanup( void )
{
  delete mRelay; mRelay = 0;
}


void KDialogBase::makeRelay( void )
{
  if( mRelay != 0 )
  {
    connect( mRelay, SIGNAL(backgroundChanged()), SLOT(updateBackground()) );
    return;
  }

  //
  // We need to protect this code because the allocation of the 
  // new KDialogBase object will trigger this method once again and the 
  // 'mRelay' will then still be 0.
  //
  static bool busy = false;
  if( busy == false )
  {
    busy = true;
    mRelay = new KDialogBase;
    atexit(cleanup);
    busy = false;
    connect( mRelay, SIGNAL(backgroundChanged()), SLOT(updateBackground()) );
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




void KDialogBase::makeButtonBox( int buttonMask, int defaultButton,
				 const QString &user1,const QString &user2,
				 const QString &user3 )
{
  //if( (buttonMask & (Cancel|Close)) == 0 ) { buttonMask |= Cancel; }
  if( buttonMask & Cancel ) { buttonMask &= ~Close; }
  if( buttonMask & Apply ) { buttonMask &= ~Try; }

  mButton.box = new QWidget( this );

  mButton.mask = buttonMask;
  if( mButton.mask & Help )
  {
    mButton.help = new QPushButton( i18n("&Help"), mButton.box );
    if( defaultButton == Help ) setButtonFocus( mButton.help, true, true );
    connect(mButton.help,SIGNAL(clicked()),SLOT(slotHelp()));
    mButton.append(mButton.help);
  }
  if( mButton.mask & Default )
  {
    mButton.def = new QPushButton( i18n("&Default"), mButton.box );
    if( defaultButton == Default ) setButtonFocus( mButton.def, true, true );
    connect(mButton.def,SIGNAL(clicked()),SLOT(slotDefault()));
    mButton.append(mButton.def);
  }
  if( mButton.mask & User3 )
  {
    mButton.user3 = new QPushButton( user3, mButton.box );
    if( defaultButton == User3 ) setButtonFocus( mButton.user3, true, true );
    connect(mButton.user3,SIGNAL(clicked()),SLOT(slotUser3()));
    mButton.append(mButton.user3);
  }
  if( mButton.mask & User2 )
  {
    mButton.user2 = new QPushButton( user2, mButton.box );
    if( defaultButton == User2 ) setButtonFocus( mButton.user2, true, true );
    connect(mButton.user2,SIGNAL(clicked()),SLOT(slotUser2()));
    mButton.append(mButton.user2);
  }
  if( mButton.mask & User1 )
  {
    mButton.user1 = new QPushButton( user1, mButton.box );
    if( defaultButton == User1 ) setButtonFocus( mButton.user1, true, true );
    connect(mButton.user1,SIGNAL(clicked()),SLOT(slotUser1()));
    mButton.append(mButton.user1);
  }

  if( mButton.mask & Ok )
  {
    mButton.ok = new QPushButton( i18n("&OK"), mButton.box );
    if( defaultButton == Ok ) setButtonFocus( mButton.ok, true, true );
    connect(mButton.ok,SIGNAL(clicked()),SLOT(slotOk()));
    mButton.append(mButton.ok);
  }
  if( mButton.mask & Apply )
  {
    mButton.apply = new QPushButton( i18n("&Apply"), mButton.box );
    if( defaultButton == Apply ) setButtonFocus( mButton.apply, true, true );
    connect(mButton.apply,SIGNAL(clicked()),SLOT(slotApply()));
    connect(mButton.apply,SIGNAL(clicked()),SLOT(applyPressed()));
    mButton.append(mButton.apply);
  }
  if( mButton.mask & Try )
  {
    mButton._try = new QPushButton( i18n("&Try"), mButton.box );
    if( defaultButton == Try ) setButtonFocus( mButton._try, true, true );
    connect(mButton._try,SIGNAL(clicked()),SLOT(slotTry()));
    mButton.append(mButton._try);
  }
  if( mButton.mask & Cancel )
  {
    mButton.cancel = new QPushButton( i18n("&Cancel"), mButton.box );
    if( defaultButton == Cancel ) setButtonFocus( mButton.cancel, true, true);
    connect(mButton.cancel,SIGNAL(clicked()),SLOT(slotCancel()));
    mButton.append(mButton.cancel);
  }
  if( mButton.mask & Close )
  {
    mButton.close = new QPushButton( i18n("&Close"), mButton.box );
    if( defaultButton == Close ) setButtonFocus( mButton.close, true, true );
    connect(mButton.close,SIGNAL(clicked()),SLOT(slotClose()));
    mButton.append(mButton.close);
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

  int numButton = 0;
  for( uint i=0; i<8; i++ )
  {
    if( i>0 && (layout[i-1]&Stretch) /*&& numButton>0*/ )
    {
      hbox->addStretch(1);
    }

    if( mButton.mask & Help & layout[i] )
    {
      hbox->addWidget( mButton.help ); numButton++;
    }
    else if( mButton.mask & Default & layout[i] )
    {
      hbox->addWidget( mButton.def ); numButton++;
    }
    else if( mButton.mask & User3 & layout[i] )
    {
      hbox->addWidget( mButton.user3 ); numButton++;
    }
    else if( mButton.mask & User2 & layout[i] )
    {
      hbox->addWidget( mButton.user2 ); numButton++;
    }
    else if( mButton.mask & User1 & layout[i] )
    {
      hbox->addWidget( mButton.user1 ); numButton++;
    }
    else if( mButton.mask & Ok & layout[i] )
    {
      hbox->addWidget( mButton.ok ); numButton++;
    }
    else if( mButton.mask & Apply & layout[i] )
    {
      hbox->addWidget( mButton.apply ); numButton++;
    }
    else if( mButton.mask & Try & layout[i] )
    {
      hbox->addWidget( mButton._try ); numButton++;
    } 
    else if( mButton.mask & Cancel & layout[i] )
    {
      hbox->addWidget( mButton.cancel ); numButton++;
    }
    else if( mButton.mask & Close & layout[i] )
    {
      hbox->addWidget( mButton.close ); numButton++;
    }
  }

  hbox->addSpacing( marginHint() ); // always
  hbox->activate();
  mButton.resize( true, marginHint(), spacingHint() );
}



void KDialogBase::enableButton( int flag, bool state )
{
  if( mButton.mask & Help & flag )
  {
    mButton.help->setEnabled( state );
  }
  if( mButton.mask & Default & flag )
  {
    mButton.def->setEnabled( state );
  }
  if( mButton.mask & User3 & flag )
  {
    mButton.user3->setEnabled( state );
  }
  if( mButton.mask & User2 & flag )
  {
    mButton.user2->setEnabled( state );
  }
  if( mButton.mask & User1 & flag )
  {
    mButton.user1->setEnabled( state );
  }
  if( mButton.mask & Ok & flag )
  {
    mButton.ok->setEnabled( state );
  }
  if( mButton.mask & Apply & flag )
  {
    mButton.apply->setEnabled( state );
  }
  if( mButton.mask & Try & flag )
  {
    mButton._try->setEnabled( state );
  }
  if( mButton.mask & Cancel & flag )
  {
   mButton.cancel->setEnabled( state ); 
  }
  if( mButton.mask & Close & flag )
  {
   mButton.close->setEnabled( state ); 
  }
}


void KDialogBase::enableButtonOK( bool state )
{
  if( mButton.mask & Ok )
  {
    mButton.ok->setEnabled( state );
  }
}


void KDialogBase::enableButtonApply( bool state )
{
  if( mButton.mask & Apply )
  {
    mButton.apply->setEnabled( state );
  }
}


void KDialogBase::enableButtonCancel( bool state )
{
  if( mButton.mask & Cancel )
  {
   mButton.cancel->setEnabled( state ); 
  }
}


void KDialogBase::showButton( int flag, bool state )
{
  if( mButton.mask & Help & flag )
  {
    state ? mButton.help->show() : mButton.help->hide();
  }
  if( mButton.mask & Default & flag )
  {
    state ? mButton.def->show() : mButton.def->hide();
  }
  if( mButton.mask & User3 & flag )
  {
    state ? mButton.user3->show() : mButton.user3->hide();
  }
  if( mButton.mask & User2 & flag )
  {
    state ? mButton.user2->show() : mButton.user2->hide();
  }
  if( mButton.mask & User1 & flag )
  {
    state ? mButton.user1->show() : mButton.user1->hide();
  }
  if( mButton.mask & Ok & flag )
  {
    state ? mButton.ok->show() : mButton.ok->hide();
  }
  if( mButton.mask & Apply & flag )
  {
    state ? mButton.apply->show() : mButton.apply->hide();
  }
  if( mButton.mask & Try & flag )
  {
    state ? mButton._try->show() : mButton._try->hide();
  }
  if( mButton.mask & Cancel & flag )
  {
    state ? mButton.cancel->show() : mButton.cancel->hide();
  }
  if( mButton.mask & Close & flag )
  {
    state ? mButton.close->show() : mButton.close->hide();
  }
}


void KDialogBase::showButtonOK( bool state )
{
  if( mButton.mask & Ok )
  {
    state ? mButton.ok->show() : mButton.ok->hide();
  }
}


void KDialogBase::showButtonApply( bool state )
{
  if( mButton.mask & Apply )
  {
    state ? mButton.apply->show() : mButton.apply->hide();
  }
}


void KDialogBase::showButtonCancel(bool state)
{
  if( mButton.mask & Cancel )
  {
    state ? mButton.cancel->show() : mButton.cancel->hide();
  }
}


void KDialogBase::setButtonOKText( const QString &text, 
				   const QString &tooltip,
				   const QString &quickhelp )
{
  if( (mButton.mask & Ok) == 0 )
  {
    return;
  }

  const QString whatsThis = i18n( ""
    "If you press the <b>OK</b> button, all changes\n"
    "you made will be used to proceed.");

  mButton.ok->setText( text == "" ? i18n("&OK") : text );
  QToolTip::add( mButton.ok, tooltip=="" ? i18n("Accept settings") : tooltip );
  QWhatsThis::add( mButton.ok, quickhelp=="" ? whatsThis : quickhelp );
}



void KDialogBase::setButtonApplyText( const QString &text, 
				      const QString &tooltip,
				      const QString &quickhelp )
{
  if( (mButton.mask & Apply) == 0 )
  {
    return;
  }

  const QString whatsThis = i18n( ""
    "When clicking <b>Apply</b>, the settings will be\n"
    "handed over to the program, but the dialog\n"
    "will not be closed."
    "Use this to try different settings. ");

  mButton.apply->setText( text == "" ? i18n("&Apply") : text );
  QToolTip::add( mButton.apply, tooltip== "" ? i18n("Apply settings"):tooltip);
  QWhatsThis::add( mButton.apply, quickhelp == "" ? whatsThis : quickhelp );
}


void KDialogBase::setButtonCancelText( const QString& text, 
				       const QString& tooltip, 
				       const QString& quickhelp )
{
  if( (mButton.mask & Cancel) == 0 )
  {
    return;
  }

  const QString whatsThis = i18n( ""
    "If you press the <b>Cancel</b> button, all changes\n"
    "you made will be abandoned and the dialog\n"
    "will be closed.\n"
    "The program will be in the state before\n"
    "opening the dialog.");

  mButton.cancel->setText( text=="" ? i18n("&Cancel") : text );
  QToolTip::add(mButton.cancel,tooltip=="" ? i18n("Cancel settings"):tooltip);
  QWhatsThis::add( mButton.cancel, quickhelp == "" ? whatsThis : quickhelp );
}



void KDialogBase::setButtonFocus( QPushButton *p,bool isDefault, bool isFocus )
{ 
   p->setDefault( isDefault );
   if( isFocus == true )
   {
     p->setFocus();
   }
   else
   {
     p->clearFocus();
   }
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


void KDialogBase::slotCancel( void ) 
{
  emit cancelClicked();
  reject();
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
      if( mButton.mask & Help )
      {
	mButton.help->animateClick();
	e->accept();
	return;
      }
    }
    if( e->key() == Key_Escape )
    {
      if( mButton.mask & Cancel )
      {
	mButton.cancel->animateClick();
	e->accept();
	return;
      }
      if( mButton.mask & Close )
      {
	mButton.close->animateClick();
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








bool KDialogBase::haveBackgroundTile( void )
{
  return( mTile == 0 ? false : true );
}


const QPixmap* KDialogBase::getBackgroundTile( void )
{
  return( mTile );
}


void KDialogBase::setBackgroundTile( const QPixmap* pix )
{
  if( pix != 0 )
  {
    if( mTile == 0 )
    {
      mTile = new QPixmap(*pix);
    }
    else
    {
      *mTile = *pix;
    }
  }
  else
  {
    delete mTile; mTile = 0;
  }

  if( mRelay != 0 )
  {
    mRelay->emitBackgroundChanged();
  }
}


void KDialogBase::updateBackground( void )
{
  if( mTile != 0 )
  {
    setBackgroundPixmap(*mTile);
    mButton.box->setBackgroundPixmap(*mTile);
    showTile( mShowTile );
  }
  else
  {
    QPixmap nullPixmap;
    setBackgroundPixmap(nullPixmap );
    mButton.box->setBackgroundPixmap(nullPixmap);      
    setBackgroundMode(PaletteBackground);
    mButton.box->setBackgroundMode(PaletteBackground);
  }
}



void KDialogBase::showTile( bool state )
{
  mShowTile = state;
  if( mShowTile == true && mTile != 0 )
  {
    setBackgroundPixmap(*mTile);
    mUrlHelp->parentWidget()->setBackgroundPixmap(*mTile);
    mButton.box->setBackgroundPixmap(*mTile);
    mUrlHelp->setTransparentMode( true );
  }
  else
  {
    setBackgroundMode(PaletteBackground);
    mUrlHelp->parentWidget()->setBackgroundMode(PaletteBackground);
    mButton.box->setBackgroundMode(PaletteBackground);
    mUrlHelp->setTransparentMode( false );
  }
}


void KDialogBase::emitBackgroundChanged( void )
{
  emit backgroundChanged() ;
}

#include "kdialogbase.moc"
