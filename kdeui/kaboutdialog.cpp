/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999 Mirko Sucker <mirko@kde.org> and 
 *  Espen Sand <espensa@online.no>
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

#include <qclipboard.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmultilineedit.h>
#include <qobjectlist.h>
#include <qpainter.h>
#include <qrect.h>


#include <kapp.h>
#include <kaboutdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktabctl.h>
#include <ktextbrowser.h>
#include <kurllabel.h>


#define WORKTEXT_IDENTATION 16
#define Grid 3

// ##############################################################
// MOC OUTPUT FILES:
#include "kaboutdialog.moc"
// ##############################################################




KAboutContributor::KAboutContributor( QWidget *parent, const char *wname,
			              const QString &name,const QString &email,
			              const QString &url, const QString &work,
			              bool showHeader, bool showFrame, 
				      bool showBold )
  : QFrame( parent, wname ), mShowHeader(showHeader), mShowBold(showBold)
{
  if( showFrame == true )
  {
    setFrameStyle(QFrame::Panel | QFrame::Raised);
  }

  mLabel[0] = new QLabel( this );
  mLabel[1] = new QLabel( this );
  mLabel[2] = new QLabel( this );
  mLabel[3] = new QLabel( this );
  mText[0] = new QLabel( this );
  mText[1] = new KURLLabel( this );
  mText[2] = new KURLLabel( this );
  mText[3] = new QLabel( this );

  setName( name, i18n("Author"), false );
  setEmail( email, i18n("Email"), false );
  setURL( url, i18n("Homepage"), false );
  setWork( work, i18n("Task"), false );

  KURLLabel *kurl = (KURLLabel*)mText[1];
  kurl->setFloat(true);
  kurl->setUnderline(true);
  connect(kurl, SIGNAL(leftClickedURL(const QString &)), 
	  SLOT(emailClickedSlot(const QString &)));

  kurl = (KURLLabel*)mText[2];
  kurl->setFloat(true);
  kurl->setUnderline(true);
  connect(kurl, SIGNAL(leftClickedURL(const QString &)), 
	  SLOT(urlClickedSlot(const QString &)));

  mLabel[3]->setAlignment( AlignTop );

  fontChange( font() );
  updateLayout();
}


void KAboutContributor::setName( const QString &text, const QString &header,
				 bool update )
{
  mLabel[0]->setText(header);
  mText[0]->setText(text);
  if( update == true ) { updateLayout(); }
}


void KAboutContributor::setEmail( const QString &text, const QString &header,
				  bool update )
{
  mLabel[1]->setText(header);
  KURLLabel *kurl = (KURLLabel*)mText[1];
  kurl->setText(text);
  kurl->setURL(text);
  if( update == true ) { updateLayout(); }
}


void KAboutContributor::setURL( const QString &text, const QString &header,
				bool update )
{
  mLabel[2]->setText(header);
  KURLLabel *kurl = (KURLLabel*)mText[2];
  kurl->setText(text);
  kurl->setURL(text);
  if( update == true ) { updateLayout(); }
}


void KAboutContributor::setWork( const QString &text, const QString &header,
				 bool update )
{
  mLabel[3]->setText(header);
  mText[3]->setText(text);
  if( update == true ) { updateLayout(); }
}


QString KAboutContributor::getName( void )
{
  return( mText[0]->text() );
}


QString KAboutContributor::getEmail( void )
{
  return( mText[1]->text() );
}


QString KAboutContributor::getURL( void )
{
  return( mText[2]->text() );
}


QString KAboutContributor::getWork( void )
{
  return( mText[3]->text() );
}



void KAboutContributor::updateLayout( void )
{
  if( layout() != 0 )
  {
    delete layout();
  }
  
  int row = 0;
  if( mText[0]->text() != QString::null ) { row += 1; }
  if( mText[1]->text() != QString::null ) { row += 1; }
  if( mText[2]->text() != QString::null ) { row += 1; }
  if( mText[3]->text() != QString::null ) { row += 1; }

  QGridLayout *gbox;
  if( row == 0 )
  {
    gbox = new QGridLayout( this, 1, 1, 0 );
    for( int i=0; i<4; i++ )
    {
      mLabel[i]->hide();
      mText[i]->hide();
    }
  }
  else
  {
    if( mText[0]->text() == QString::null && mShowHeader == false )
    {
      gbox = new QGridLayout( this, row, 1, frameWidth()+1, 2 );
    }
    else
    {
      gbox = new QGridLayout( this, row, 2, frameWidth()+1, 2 );
      if( mShowHeader == false )
      {
	gbox->addColSpacing( 0, KDialog::spacingHint()*2 );
      }
      gbox->setColStretch( 1, 10 );
    }

    for( int i=0, r=0; i<4; i++ )
    {
      mLabel[i]->setFixedHeight( fontMetrics().lineSpacing() );
      if( i != 3 )
      {
	mText[i]->setFixedHeight( fontMetrics().lineSpacing() );
      }

      if( mText[i]->text() != QString::null )
      {
	if( mShowHeader == true )
	{
	  gbox->addWidget( mLabel[i], r, 0, AlignLeft );
	  gbox->addWidget( mText[i], r, 1, AlignLeft  );
	  mLabel[i]->show();
	  mText[i]->show();
	}
	else
	{
	  mLabel[i]->hide();
	  if( i == 0 )
	  {
	    gbox->addMultiCellWidget( mText[i], r, r, 0, 1, AlignLeft );
	  }
	  else
	  {
	    gbox->addWidget( mText[i], r, 1, AlignLeft  );
	  }
	  mText[i]->show();
	}
	r++;
      }
      else
      {
	mLabel[i]->hide();
	mText[i]->hide();
      }
    }
  }

  gbox->activate();
  setMinimumSize( sizeHint() );
}


void KAboutContributor::fontChange( const QFont &oldFont )
{
  if( mShowBold == true )
  {
    QFont f( font() );
    f.setBold( true );
    mText[0]->setFont( f );
  }
  update();
}


QSize KAboutContributor::sizeHint( void )
{
  return( minimumSizeHint() );
}


void KAboutContributor::urlClickedSlot( const QString &u )
{
  emit openURL(u);
}


void KAboutContributor::emailClickedSlot( const QString &e )
{
  emit sendEmail( mText[0]->text(), e ) ;
}


//
// Internal widget for the KAboutDialog class. 
//
KAboutContainerBase::KAboutContainerBase( int layoutType, QWidget *parent,
					  char *name )
  : QWidget( parent, name ),
    mTitleLabel(0),mIconLabel(0),mVersionLabel(0),mAuthorLabel(0),
    mImageFrame(0),mImageLabel(0),mPageTab(0),mPlainSpace(0)
{
  mTopLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );
  if( mTopLayout == 0 ) { return; }

  if( layoutType & AbtImageOnly ) 
  { 
    layoutType &= ~(AbtImageLeft|AbtImageRight|AbtTabbed|AbtPlain);
  }
  if( layoutType & AbtImageLeft ) 
  { 
    layoutType &= ~AbtImageRight; 
  }
   
  if( layoutType & AbtTitle )
  {
    mTitleLabel = new QLabel( this, "title" );
    mTitleLabel->setAlignment(AlignCenter);
    mTopLayout->addWidget( mTitleLabel );
    mTopLayout->addSpacing( KDialog::spacingHint() );
  }

  if( layoutType & AbtProduct )
  {
    QWidget *productArea = new  QWidget( this, "area" );
    mTopLayout->addWidget( productArea, 0, AlignLeft );
    
    QHBoxLayout *hbox = new QHBoxLayout(productArea,0,KDialog::spacingHint());
    if( hbox == 0 ) { return; }
   
    mIconLabel = new QLabel( productArea );
    hbox->addWidget( mIconLabel );
   
    QVBoxLayout *vbox = new QVBoxLayout();
    if( vbox == 0 ) { return; }
    hbox->addLayout( vbox );

    mVersionLabel = new QLabel( productArea, "version" );
    mAuthorLabel  = new QLabel( productArea, "author" );
    vbox->addWidget( mVersionLabel );
    vbox->addWidget( mAuthorLabel );
    hbox->activate();

    mTopLayout->addSpacing( KDialog::spacingHint() );
  }

  QHBoxLayout *hbox = new QHBoxLayout();
  if( hbox == 0 ) { return; }
  mTopLayout->addLayout( hbox, 10 );

  if( layoutType & AbtImageLeft )
  {
    QVBoxLayout *vbox = new QVBoxLayout();
    hbox->addLayout(vbox);
    vbox->addSpacing(1);
    mImageFrame = new QFrame( this );
    setImageFrame( true );
    vbox->addWidget( mImageFrame );
    vbox->addSpacing(1);
  
    vbox = new QVBoxLayout( mImageFrame, 1 );
    mImageLabel = new KImageTrackLabel( mImageFrame );
    connect( mImageLabel, SIGNAL(mouseTrack( int, const QMouseEvent * )),
	     SLOT( slotMouseTrack( int, const QMouseEvent * )) );
    vbox->addStretch(10);
    vbox->addWidget( mImageLabel );
    vbox->addStretch(10);
    vbox->activate();
  }

  if( layoutType & AbtTabbed )
  {
    mPageTab = new KTabCtl( this );
    if( mPageTab == 0 ) { return; }
    hbox->addWidget( mPageTab, 10 );
  }
  else if( layoutType & AbtImageOnly )
  {
    mImageFrame = new QFrame( this );
    setImageFrame( true );
    hbox->addWidget( mImageFrame, 10 );

    QGridLayout *gbox = new QGridLayout(mImageFrame, 3, 3, 1, 0 );    
    gbox->setRowStretch( 0, 10 );
    gbox->setRowStretch( 2, 10 );
    gbox->setColStretch( 0, 10 );
    gbox->setColStretch( 2, 10 );
 
    mImageLabel = new KImageTrackLabel( mImageFrame );
    connect( mImageLabel, SIGNAL(mouseTrack( int, const QMouseEvent * )),
	     SLOT( slotMouseTrack( int, const QMouseEvent * )) );
    gbox->addWidget( mImageLabel, 1, 1 );
    gbox->activate();
  }
  else
  {
    mPlainSpace = new QFrame( this );
    if( mPlainSpace == 0 ) { return; }
    hbox->addWidget( mPlainSpace, 10 );
  }

  if( layoutType & AbtImageRight )
  {
    QVBoxLayout *vbox = new QVBoxLayout();
    hbox->addLayout(vbox);
    vbox->addSpacing(1);
    mImageFrame = new QFrame( this );
    setImageFrame( true );
    vbox->addWidget( mImageFrame );
    vbox->addSpacing(1);
  
    vbox = new QVBoxLayout( mImageFrame, 1 );
    mImageLabel = new KImageTrackLabel( mImageFrame );
    connect( mImageLabel, SIGNAL(mouseTrack( int, const QMouseEvent * )),
	     SLOT( slotMouseTrack( int, const QMouseEvent * )) );
    vbox->addStretch(10);
    vbox->addWidget( mImageLabel );
    vbox->addStretch(10);
    vbox->activate();
  }

  fontChange( font() );
}


void KAboutContainerBase::show( void )
{
  QWidget::show();

  if( mPageTab != 0 )
  {
    mPageTab->setMinimumSize( mPageTab->sizeHint() );
  }
  mTopLayout->activate(); // This must be done after everything else.
}


QSize KAboutContainerBase::sizeHint( void )
{
  return( minimumSize() );
}


void KAboutContainerBase::fontChange( const QFont &oldFont )
{
  if( mTitleLabel != 0 )
  {
    QFont f( KGlobal::generalFont() );
    f.setBold( true );
    f.setPointSize( 14 ); // Perhaps changeable ?
    mTitleLabel->setFont(f);
  }

  if( mVersionLabel != 0 )
  {
    QFont f( KGlobal::generalFont() );
    f.setBold( true );
    mVersionLabel->setFont(f);
    mAuthorLabel->setFont(f);
    mVersionLabel->parentWidget()->layout()->activate();
  }

  update();
}






QFrame *KAboutContainerBase::addTextPage( const QString &title, 
					  const QString &text, 
					  bool richText, int numLines )
{
  QFrame *page = addEmptyPage( title );
  if( page == 0 ) { return(0); }
  if( numLines <= 0 ) { numLines = 10; }

  QVBoxLayout *vbox = new QVBoxLayout( page, KDialog::spacingHint() );

  if( richText == true )
  {
    KTextBrowser *browser = new KTextBrowser( page, "browser" );  
    browser->setHScrollBarMode( QScrollView::AlwaysOff );
    browser->setText( text );
    browser->setMinimumHeight( fontMetrics().lineSpacing()*numLines );

    vbox->addWidget(browser);
    connect(browser, SIGNAL(urlClick(const QString &)), 
	    SLOT(slotUrlClick(const QString &)));
    connect(browser, SIGNAL(mailClick(const QString &,const QString &)), 
	    SLOT(slotMailClick(const QString &,const QString &)));
  }
  else
  {
    QMultiLineEdit *lineEdit = new QMultiLineEdit( page, "text" );
    lineEdit->setReadOnly( true );
    lineEdit->setText( text );
    lineEdit->setMinimumWidth( lineEdit->maxLineWidth() + 20 );
    lineEdit->setMinimumHeight( fontMetrics().lineSpacing()*numLines );
    vbox->addWidget( lineEdit );
  }

  page->setMinimumSize( page->sizeHint() );  
  vbox->activate();
  return( page );
}


KAboutContainer *KAboutContainerBase::addContainerPage( const QString &title, 
							int childAlignment,
							int innerAlignment ) 
{
  if( mPageTab == 0 )
  {
    cout << "addPage: " << "Invalid layout" << endl;
    return( 0 );
  }
  
  KAboutContainer *container = new KAboutContainer( mPageTab, "container",
    KDialog::spacingHint(), KDialog::spacingHint(), childAlignment, 
						  innerAlignment );
  mPageTab->addTab( container, title );

  if( mContainerList.resize( mContainerList.size() + 1) == true )
  {
    mContainerList[ mContainerList.size()-1 ]=container;
  }

  connect(container, SIGNAL(urlClick(const QString &)), 
	  SLOT(slotUrlClick(const QString &)));
  connect(container, SIGNAL(mailClick(const QString &,const QString &)), 
	  SLOT(slotMailClick(const QString &,const QString &)));

  return( container );
}


QFrame *KAboutContainerBase::addEmptyPage( const QString &title )
{
  if( mPageTab == 0 )
  {
    cout << "addPage: " << "Invalid layout" << endl;
    return( 0 );
  }
  
  QFrame *page = new QFrame( mPageTab, title );
  page->setFrameStyle( QFrame::NoFrame );

  mPageTab->addTab( page, title );
  return( page );
}


KAboutContainer *KAboutContainerBase::addContainer( int childAlignment,
						    int innerAlignment ) 
{
  KAboutContainer *container = new KAboutContainer( this, "container",
    0, KDialog::spacingHint(), childAlignment, innerAlignment );
  mTopLayout->addWidget( container, 0, childAlignment );

  if( mContainerList.resize( mContainerList.size() + 1) == true )
  {
    mContainerList[ mContainerList.size()-1 ]=container;
  }

  connect(container, SIGNAL(urlClick(const QString &)), 
	  SLOT(slotUrlClick(const QString &)));
  connect(container, SIGNAL(mailClick(const QString &,const QString &)), 
	  SLOT(slotMailClick(const QString &,const QString &)));

  return( container );
}



void KAboutContainerBase::setTitle( const QString &title )
{
  if( mTitleLabel == 0 )
  {
    cout << "setTitle: " << "Invalid layout" << endl;
    return;
  }  
  mTitleLabel->setText(title);
}


void KAboutContainerBase::setImage( const char *fileName )
{
  if( mImageLabel == 0 )
  {
    cout << "setImage: " << "Invalid layout" << endl;
    return;
  }
  if( fileName == 0 )
  {
    return;
  }

  QImage logo( fileName );
  if( logo.isNull() == false )
  {
    QPixmap pix;
    pix = logo;
    mImageLabel->setPixmap( pix );
  }
  mImageFrame->layout()->activate();

}


void KAboutContainerBase::setImageBackgroundColor( const QColor &color )
{
  if( mImageFrame != 0 )
  {
    mImageFrame->setBackgroundColor( color );
  }
}


void KAboutContainerBase::setImageFrame( bool state )
{
  if( mImageFrame != 0 )
  {
    if( state == true )
    {
      mImageFrame->setFrameStyle( QFrame::Panel | QFrame::Plain );
      mImageFrame->setLineWidth(1);
    }
    else
    {
      mImageFrame->setFrameStyle( QFrame::NoFrame );
      mImageFrame->setLineWidth(0);
    }
  }
}


void KAboutContainerBase::setProduct( const QString &appName, 
				      const QString &version,
				      const QString &author, 
				      const QString &year )
{
  if( mIconLabel == 0 )
  {
    cout << "setProduct: " << "Invalid layout" << endl;
    return;
  }
  
  mIconLabel->setPixmap( kapp->getIcon() );

  QString msg1 = i18n("Version: %1 %2 (KDE %3)").arg(appName).arg(version).
    arg(KDE_VERSION_STRING);
  QString msg2 = i18n("%1 %2, %3").arg('©').arg(year).arg(author);

  mVersionLabel->setText( msg1 );
  mAuthorLabel->setText( msg2 );
  mIconLabel->parentWidget()->layout()->activate();
}


void KAboutContainerBase::slotMouseTrack( int mode, const QMouseEvent *e )
{
  emit mouseTrack( mode, e );
}


void KAboutContainerBase::slotUrlClick( const QString &url )
{
  emit urlClick( url );
}

void KAboutContainerBase::slotMailClick( const QString &name, 
					 const QString &address )
{
  emit mailClick( name, address );
}



KAboutContainer::KAboutContainer( QWidget *parent, const char *name, 
				  int margin, int spacing,
				  int childAlignment, int innerAlignment )
  : QFrame( parent, name )
{
  mAlignment = innerAlignment;

  QGridLayout *gbox = new QGridLayout( this, 3, 3, margin, spacing );
  if( childAlignment & AlignHCenter )
  {
    gbox->setColStretch( 0, 10 );
    gbox->setColStretch( 2, 10 );
  }
  else if( childAlignment & AlignRight )
  {  
    gbox->setColStretch( 0, 10 );
  }
  else
  {
    gbox->setColStretch( 2, 10 );
  }

  if( childAlignment & AlignVCenter )
  {
    gbox->setRowStretch( 0, 10 );
    gbox->setRowStretch( 2, 10 );
  }
  else if( childAlignment & AlignRight )
  {  
    gbox->setRowStretch( 0, 10 );
  }
  else
  {
    gbox->setRowStretch( 2, 10 );
  }

  mVbox = new QVBoxLayout( spacing );
  gbox->addLayout( mVbox, 1, 1 );
  gbox->activate();
}


void KAboutContainer::childEvent( QChildEvent *e )
{
  if( !e->inserted() || !e->child()->isWidgetType() )
  {
    return;
  }

  QWidget *w = (QWidget*)e->child();
  mVbox->addWidget( w, 0, mAlignment );
  QSize s( sizeHint() );
  setMinimumSize( s );

  QObjectList *l = (QObjectList*)children(); // silence please
  for( uint i=0; i < l->count(); i++ )
  {
    QObject *o = l->at(i);
    if( o->isWidgetType() )
    {
      ((QWidget*)o)->setMinimumWidth( s.width() );
    }
  }
}


QSize KAboutContainer::sizeHint( void ) const
{
  //
  // The size is computed by adding the sizeHint().height() of all 
  // widget children and taking the width of the widest child and adding
  // layout()->margin() and layout()->spacing()
  //

  QSize size;

  int numChild = 0;
  QObjectList *l = (QObjectList*)children(); // silence please

  for( uint i=0; i < l->count(); i++ )
  {
    QObject *o = l->at(i);
    if( o->isWidgetType() )
    {
      numChild += 1;
      QWidget *w=((QWidget*)o);

      QSize s = w->minimumSize();
      if( s.isEmpty() == true )
      {
	s = w->minimumSizeHint();
	if( s.isEmpty() == true )
	{
	  s = w->sizeHint();
	  if( s.isEmpty() == true )
	  {
	    s = QSize( 100, 100 ); // Default size
	  }
	}
      }
      size.setHeight( size.height() + s.height() );
      if( s.width() > size.width() ) { size.setWidth( s.width() ); }
    }
  }

  if( numChild > 0 ) 
  { 
    //
    // Seems I have to add 1 to the height to properly show the border
    // of the last entry if layout()->margin() is 0
    //

    size.setHeight( size.height() + layout()->spacing()*(numChild-1) );
    size += QSize( layout()->margin()*2, layout()->margin()*2 + 1 );
  }
  else
  {
    size = QSize( 1, 1 );
  }
  return( size );
}


QSize KAboutContainer::minimumSizeHint( void ) const
{
  return( sizeHint() );
}


void KAboutContainer::addWidget( QWidget *widget )
{
  widget->recreate( this, 0, QPoint(0,0) );
}


void KAboutContainer::addPerson( const QString &name, const QString &email,
				 const QString &url, const QString &task,
				 bool showHeader, bool showFrame,bool showBold)
{

  KAboutContributor *cont = new KAboutContributor( this, "pers",
    name, email, url, task, showHeader, showFrame, showBold ); 
  connect( cont, SIGNAL( openURL(const QString&)),
	   this, SIGNAL( urlClick(const QString &))); 
  connect( cont, SIGNAL( sendEmail(const QString &, const QString &)),
	   this, SIGNAL( mailClick(const QString &, const QString &)));
}


void KAboutContainer::addTitle( const QString &title, int alignment, 
				bool showFrame, bool showBold )
{

  QLabel *label = new QLabel( title, this, "title" );
  if( showBold == true )
  {
    QFont labelFont( font() );
    labelFont.setBold( true );
    label->setFont( labelFont );
  }
  if( showFrame == true )
  {
    label->setFrameStyle(QFrame::Panel | QFrame::Raised);
  }
  label->setAlignment( alignment );
}


void KAboutContainer::addImage( const char *fileName, int alignment )
{
  if( fileName == 0 )
  {
    return;
  }

  KImageTrackLabel *label = new KImageTrackLabel( this, "image" );
  QImage logo( fileName );
  if( logo.isNull() == false )
  {
    QPixmap pix;
    pix = logo;
    label->setPixmap( pix );
  }
  label->setAlignment( alignment );
}





















#if 0


/** Every person displayed is stored in a KAboutContributor object.
 *  Every contributor, the author and/or the maintainer of the application are
 *  stored in objects of this local class. Every single field may be empty. 
 *  To add a contributor, create a KAboutContributor object as a child of your
 *  @ref KAboutDialog, set its contents and add it using add addContributor. */
class KAboutContributor : public QFrame
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  /** The Qt constructor. */
  KAboutContributor(QWidget* parent=0, const char* name=0);
  /** Set the name (a literal string). */
  void setName(const QString&);
  /** Get the name. */
  QString getName();
  /** The email address (dito). */
  void setEmail(const QString&);
  /** Get the email address. */
  QString getEmail();
  /** The URL (dito). */
  void setURL(const QString&);
  /** Get the URL. */
  QString getURL();
  /** The tasks the person worked on (a literal string). More than one line is 
   *  possible, but very long texts might look ugly. */
  void setWork(const QString&);
  /** The size hint. Very important here, since KAboutWidget relies on it for
   *  geometry management. */
  QSize sizeHint();
  QSize minimumSizeHint(void);
  virtual void show( void );  

  // ----------------------------------------------------------------------------
protected:
  // events:
  /** The resize event. */
  void resizeEvent(QResizeEvent*);
  /** The paint event. */
  void paintEvent(QPaintEvent*);
  /** The label showing the program version. */
  QLabel *name;
  /** The clickable URL label showing the email address. It is only visible if
   *  its text is not empty. */
  KURLLabel *email;
  /** Another interactive part that displays the homepage URL. */
  KURLLabel *url;
  /** The description of the contributions of the person. */
  QString work;
  // ----------------------------------------------------------------------------
protected slots:
  /** The homepage URL has been clicked. */
  void urlClickedSlot(const QString&);
  /** The email address has been clicked. */
  void emailClickedSlot(const QString& emailaddress);
  // ----------------------------------------------------------------------------
signals:
  /** The email address has been clicked. */
  void sendEmail(const QString& name, const QString& email);
  /** The URL has been clicked. */
  void openURL(const QString& url);
  // ############################################################################
};



KAboutContributor::KAboutContributor(QWidget* parent, const char* n)
  : QFrame(parent, n),
    name(new QLabel(this)),
    email(new KURLLabel(this)),
    url(new KURLLabel(this))
{
  // ############################################################
  if(name==0 || email==0)
    { // this will nearly never happen (out of memory in about box?)
      debug("KAboutContributor::KAboutContributor: Out of memory.");
      kapp->quit();
    }
  setFrameStyle(QFrame::Panel | QFrame::Raised);
  // -----
  connect(email, SIGNAL(leftClickedURL(const QString&)),
	  SLOT(emailClickedSlot(const QString&)));
  connect(url, SIGNAL(leftClickedURL(const QString&)),
	  SLOT(urlClickedSlot(const QString&)));
  // ############################################################
}

void
KAboutContributor::setName(const QString& n)
{
  // ############################################################
  name->setText(n);
  // ############################################################
}

QString
KAboutContributor::getName()
{
  // ###########################################################
  return name->text();
  // ###########################################################
}
void
KAboutContributor::setURL(const QString& u)
{
  // ###########################################################
  url->setText(u);
  // ###########################################################
}

QString
KAboutContributor::getURL()
{
  // ###########################################################
  return url->text();
  // ###########################################################
}

void
KAboutContributor::setEmail(const QString& e)
{
  // ###########################################################
  email->setText(e);
  // ###########################################################
}

QString
KAboutContributor::getEmail()
{
  // ###########################################################
  return email->text();
  // ###########################################################
}

void
KAboutContributor::emailClickedSlot(const QString& e)
{
  // ###########################################################
  debug("KAboutContributor::emailClickedSlot: called.");
  emit(sendEmail(name->text(), e));
  // ###########################################################
}

void
KAboutContributor::urlClickedSlot(const QString& u)
{
  // ###########################################################
  debug("KAboutContributor::urlClickedSlot: called.");
  emit(openURL(u));
  // ###########################################################
}

void
KAboutContributor::setWork(const QString& w)
{
  // ###########################################################
  work=w;
  // ###########################################################
}

#endif


#if 0
QSize
KAboutContributor::sizeHint()
{ 
  // ############################################################################
  const int FrameWidth=frameWidth();
  const int WorkTextWidth=200;
  int maxx, maxy;
  QRect rect;
  // ----- first calculate name and email width: 
  maxx=name->sizeHint().width();
  maxx=QMAX(maxx, email->sizeHint().width()+WORKTEXT_IDENTATION);
  // ----- now determine "work" text rectangle:
  if(!work.isEmpty()) // save time
    {
      rect=fontMetrics().boundingRect
	(0, 0, WorkTextWidth, 32000, WordBreak | AlignLeft, work);
    }
  if(maxx<rect.width())
  {
    maxx=WorkTextWidth+WORKTEXT_IDENTATION;
  }
  maxx=QMAX(maxx, url->sizeHint().width()+WORKTEXT_IDENTATION);
  // -----
  maxy=2*(name->sizeHint().height()+Grid); // need a space above the KURLLabels
  maxy+=/* email */ name->sizeHint().height();
  maxy+=rect.height();
  // -----
  maxx+=2*FrameWidth;
  maxy+=2*FrameWidth;
  return QSize(maxx, maxy);
  // ############################################################################
}

QSize KAboutContributor::minimumSizeHint(void)
{
  return( sizeHint() );
}


void KAboutContributor::show( void )
{
  QFrame::show();
  setMinimumSize( sizeHint() );
}



void
KAboutContributor::resizeEvent(QResizeEvent*)
{ // the widgets are simply aligned from top to bottom, since the parent is
  // expected to respect the size hint
  // ############################################################################
  int framewidth=frameWidth(), childwidth=width()-2*framewidth;
  int cy=framewidth;
  // -----
  name->setGeometry
    (framewidth, framewidth, childwidth, name->sizeHint().height());
  cy=name->height()+Grid;
  email->setGeometry
    (framewidth+WORKTEXT_IDENTATION, cy,
     childwidth-WORKTEXT_IDENTATION, /* email */ name->sizeHint().height());
  cy+=name->height()+Grid;
  url->setGeometry
    (framewidth+WORKTEXT_IDENTATION, cy,
     childwidth-WORKTEXT_IDENTATION, /* url */ name->sizeHint().height());
  // the work text is drawn in the paint event
  // ############################################################################
}


void
KAboutContributor::paintEvent(QPaintEvent* e)
{ // the widgets are simply aligned from top to bottom, since the parent is
  // expected to respect the size hint (the widget is only used locally by now)
  // ############################################################################
  int cy=frameWidth()+name->height()+email->height()+Grid+url->height()+Grid;
  int h=height()-cy-frameWidth();
  int w=width()-WORKTEXT_IDENTATION-2*frameWidth();
  // -----
  QFrame::paintEvent(e);
  if(work.isEmpty()) return;
  QPainter paint(this); // construct painter only if there is something to draw
  // -----
  paint.drawText(WORKTEXT_IDENTATION, cy, w, h, AlignLeft | WordBreak, work);
  // ############################################################################
}
#endif


#if 0
QSize KAboutContributor::sizeHint( void )
{ 
  int s = KDialog::spacingHint();
  int h = fontMetrics().lineSpacing()*3 + 2*s;
  int m = frameWidth();

  int w = name->sizeHint().width();
  w = QMAX( w, email->sizeHint().width()+s);
  w = QMAX( w, url->sizeHint().width()+s);
  
  if( work.isEmpty() == false )
  {
    const int WorkTextWidth=200;
    QRect r = fontMetrics().boundingRect
      (0, 0, WorkTextWidth, 32000, WordBreak | AlignLeft, work);
    if( w < r.width() )
    {
      w = QMAX( w, WorkTextWidth+s );
    }
    h += QMAX( fontMetrics().lineSpacing(), r.height() ) + s;
  }
  return( QSize( w + 2*m, h + 2*m ) );


  /*
  int s = 3;
  int m = frameWidth() + KDialog::spacingHint();
  int h = ls * 3 + s * 2;
  int w = name->sizeHint().width();

  w = QMAX( w, email->sizeHint().width()+WORKTEXT_IDENTATION);
  w = QMAX( w, url->sizeHint().width()+WORKTEXT_IDENTATION);
  if( work.isEmpty() == false )
  {
    const int WorkTextWidth=200;

    QRect r = fontMetrics().boundingRect
      (0, 0, WorkTextWidth, 32000, WordBreak | AlignLeft, work);
    if( w < r.width() )
    {
      w = QMAX( w, WorkTextWidth + WORKTEXT_IDENTATION );
    }
    h += r.height() + s;
  }
  return( QSize( w + 2*m, h + 2*m ) );
  */
}


//
// The widgets are simply aligned from top to bottom, since the parent is
// expected to respect the size hint
//
void KAboutContributor::resizeEvent(QResizeEvent*)
{
  int x = frameWidth();
  int s = KDialog::spacingHint();
  int h = fontMetrics().lineSpacing();
  int w = width() - 2*x;
  int y = x;
  
  name->setGeometry( x, y, w, h );
  y += h + s;
  email->setGeometry( x+s, y, w-s, h );
  y += h + s;
  url->setGeometry( x+s, y, w-s, h );

  /*
  int x = frameWidth() + KDialog::spacingHint();
  int y = x;
  int w = width() - 2*x;
  int h = name->sizeHint().height();
  int s = 3;

  name->setGeometry( x, y, w, h );
  y += h + s;
  email->setGeometry( x+WORKTEXT_IDENTATION, y, w-WORKTEXT_IDENTATION, h );
  y += h + s;
  url->setGeometry( x+WORKTEXT_IDENTATION, y, w-WORKTEXT_IDENTATION, h );
  //
  // the work text is drawn in the paint event
  //
  */
}



void KAboutContributor::paintEvent( QPaintEvent *e )
{
  QFrame::paintEvent(e);
  if(work.isEmpty()) return;

  int x = frameWidth() + KDialog::spacingHint();
  int h = fontMetrics().lineSpacing();
  int y = height() - frameWidth() - fontMetrics().lineSpacing();
  int w = width() - frameWidth()*2 - KDialog::spacingHint();
  
  QPainter paint( this );
  paint.drawText( x, y, w, h, AlignLeft | WordBreak, work );

  /*
 
  int s = 3;
  int x = frameWidth() + KDialog::spacingHint() + WORKTEXT_IDENTATION;
  int w = width()-WORKTEXT_IDENTATION-2*(frameWidth()+KDialog::spacingHint());
  int y = frameWidth()+KDialog::spacingHint()+(name->sizeHint().height()+s)*3;
  int h = height()-y-frameWidth();

  QPainter paint( this );
  paint.drawText( x, y, w, h, AlignLeft | WordBreak, work );
  */
}
#endif






KAboutWidget::KAboutWidget(QWidget* parent, const char* name)
  : QWidget(parent, name),
    version(new QLabel(this)),
    cont(new QLabel(this)),
    logo(new QLabel(this)),
    author(new KAboutContributor(this)),
    maintainer(new KAboutContributor(this)),
    showMaintainer(false)
{
  // #################################################################
  if( version==0 || cont==0 || logo==0 || author==0 || maintainer==0 )
  { 
    // this will nearly never happen (out of memory in about box?)
    debug("KAboutWidget::KAboutWidget: Out of memory.");
    kapp->quit();
  }
  // -----
  cont->setText(i18n("Other Contributors:"));
  logo->setText(i18n("(No logo available)"));
  logo->setFrameStyle(QFrame::Panel | QFrame::Raised);
  version->setAlignment(AlignCenter);
  // -----
  connect(author, SIGNAL(sendEmail(const QString&, const QString&)),
	  SLOT(sendEmailSlot(const QString&, const QString&)));
  connect(author, SIGNAL(openURL(const QString&)),
	  SLOT(openURLSlot(const QString&)));
  connect(maintainer, SIGNAL(sendEmail(const QString&, const QString&)),
	  SLOT(sendEmailSlot(const QString&, const QString&)));
  connect(maintainer, SIGNAL(openURL(const QString&)),
	  SLOT(openURLSlot(const QString&)));
  // #################################################################
}


void
KAboutWidget::adjust()
{
  // #################################################################
  int cx, cy, tempx;
  list<KAboutContributor*>::iterator pos;
  int maintWidth, maintHeight;
  QSize size;
  // -----
  if(showMaintainer)
    {
      size=maintainer->sizeHint();
      maintWidth=size.width();
      maintHeight=size.height();
    } else {
      maintWidth=0;
      maintHeight=0;
    }
  size=author->sizeHint();
  logo->adjustSize();
  cy=version->sizeHint().height()+Grid;
  cx=logo->width();
  tempx=QMAX(size.width(), maintWidth);
  cx+=Grid+tempx;
  cx=QMAX(cx, version->sizeHint().width());
  cy+=QMAX(logo->height(),
	   size.height()+(showMaintainer ? Grid+maintHeight : 0));
  // -----
  if(!contributors.empty())
    {
      cx=QMAX(cx, cont->sizeHint().width());
      cy+=cont->sizeHint().height()+Grid;
      for(pos=contributors.begin(); pos!=contributors.end(); ++pos)
	{
	  cy+=(*pos)->sizeHint().height();
	}
    }
  // -----
  setMinimumSize(cx, cy);
  // #################################################################
}

void 
KAboutWidget::setLogo(const QPixmap& i)
{
  // ############################################################################
  logo->setPixmap(i);
  // ############################################################################
}

void KAboutWidget::sendEmailSlot(const QString& name, const QString& email)
{
  emit(sendEmail(name, email));
}

void KAboutWidget::openURLSlot(const QString& url)
{
  emit(openURL(url));
}

void
KAboutWidget::setAuthor(const QString& name, const QString& email,
			const QString& url, const QString& w)
{
  // ############################################################################
  author->setName(name);
  author->setEmail(email);
  author->setURL(url);
  author->setWork(w);
  // ############################################################################
}

void
KAboutWidget::setMaintainer(const QString& name, const QString& email,
			    const QString& url, const QString& w)
{
  // ############################################################################
  maintainer->setName(name);
  maintainer->setEmail(email);
  maintainer->setWork(w);
  maintainer->setURL(url);
  showMaintainer=true;
  // ############################################################################
}

void
KAboutWidget::addContributor(const QString& n, const QString& e,
			     const QString& url, const QString& w)
{
  // ############################################################################
  KAboutContributor *c=new KAboutContributor(this);
  // -----
  c->setName(n);
  c->setEmail(e);
  c->setURL(url);
  c->setWork(w);
  contributors.push_back(c);
  connect(c, SIGNAL(sendEmail(const QString&, const QString&)),
	  SLOT(sendEmailSlot(const QString&, const QString&)));
  connect(c, SIGNAL(openURL(const QString&)), SLOT(openURLSlot(const QString&)));
  // ############################################################################
}

void
KAboutWidget::setVersion(const QString& name)
{
  // ############################################################################
  version->setText(name);
  // ############################################################################
}

void
KAboutWidget::resizeEvent(QResizeEvent*)
{
  // ############################################################################
  int x, y, cx, tempx, tempy;
  list<KAboutContributor*>::iterator pos;
  // -----
  x=0;
  // ----- set version label geometry:
  version->setGeometry(0, 0, width(), version->sizeHint().height());
  y=version->height()+Grid;
  // ----- move logo to correct position:
  logo->adjustSize();
  logo->move(0, y);
  // ----- move author and maintainer right to it:
  tempx=logo->width()+Grid;
  cx=width()-tempx;
  author->setGeometry
    (tempx, y, cx, author->sizeHint().height());
  maintainer->setGeometry
    (tempx, y+author->height()+Grid, cx, maintainer->sizeHint().height());

  y+=QMAX(logo->height(),
	  author->height()+(showMaintainer ? Grid+maintainer->height() : 0));
  // -----
  if(!contributors.empty())
    {
      tempy=cont->sizeHint().height();
      cont->setGeometry(0, y, width(), tempy);
      cont->show();
      y+=tempy+Grid;
    } else {
      cont->hide();
    }
  for(pos=contributors.begin(); pos!=contributors.end(); ++pos)
    {
      tempy=(*pos)->sizeHint().height();
      // y+=Grid;
      (*pos)->setGeometry(x, y, width(), tempy);
      y+=tempy;
    }
  if(showMaintainer)
    {
      maintainer->show();
    } else {
      maintainer->hide();
    }
  // ############################################################################
}

KAboutDialog::KAboutDialog(QWidget* parent, const char* name, bool modal)
  : KDialogBase(parent, name, modal, QString::null, Ok, Ok ),
    about(new KAboutWidget(this)), mContainerBase(0)
{
  // #################################################################
  if(about==0)
  { 
    // this will nearly never happen (out of memory in about box?)
    debug("KAboutDialog::KAboutDialog: Out of memory.");
    kapp->quit();
  }
  setMainWidget(about);
  connect(about, SIGNAL(sendEmail(const QString&, const QString&)),
	  SLOT(sendEmailSlot(const QString&, const QString&)));
  connect(about, SIGNAL(openURL(const QString&)), 
	  SLOT(openURLSlot(const QString&)));
  // #################################################################
}


KAboutDialog::KAboutDialog( int layoutType, const QString &caption,
			    int buttonMask, int defaultButton,
			    QWidget *parent, const char *name, bool modal, 
			    bool separator, const QString &user1, 
			    const QString &user2, const QString &user3 )
  :KDialogBase( parent, name, modal, QString::null, buttonMask, defaultButton,
		separator, user1, user2, user3 ), 
   about(0)
{
  setPlainCaption( i18n("About %1").arg(caption) );

  mContainerBase = new KAboutContainerBase( layoutType, this );
  setMainWidget(mContainerBase);

  connect( mContainerBase, SIGNAL(urlClick(const QString &)),
	   this, SLOT(openURLSlot(const QString &)));
  connect( mContainerBase, SIGNAL(mailClick(const QString &,const QString &)),
	   this, SLOT(sendEmailSlot(const QString &,const QString &)));
  connect( mContainerBase, SIGNAL(mouseTrack(int, const QMouseEvent *)),
	   this, SLOT(mouseTrackSlot(int, const QMouseEvent *)));
}


void KAboutDialog::show( void )
{
  adjust();
  if( mContainerBase != 0 ) { mContainerBase->show(); }
  KDialogBase::show();
}


void KAboutDialog::show( QWidget *centerParent )
{
  adjust();
  if( mContainerBase != 0 ) { mContainerBase->show(); }
  KDialogBase::show( centerParent );
}


void KAboutDialog::adjust()
{
  if( about == 0 ) { return; }
  about->adjust();
  initializeGeometry();
  resize(minimumSize());
}


void KAboutDialog::setLogo(const QPixmap& i)
{
  if( about == 0 ) { return; }
  about->setLogo(i);
}


void KAboutDialog::setMaintainer(const QString& name, const QString& email,
				 const QString& url, const QString& w)
{
  // #################################################################
  if( about == 0 ) { return; }
  about->setMaintainer(name, email, url, w);
  // #################################################################
}

void KAboutDialog::setAuthor(const QString& name, const QString& email,
			     const QString& url, const QString& work)
{
  // #################################################################
  if( about == 0 ) { return; }
  about->setAuthor(name, email, url, work);
  // #################################################################
}

void KAboutDialog::addContributor(const QString& n, const QString& e,
				  const QString& u, const QString& w)
{
  // #################################################################
  if( about == 0 ) { return; }
  about->addContributor(n, e, u, w);
  // #################################################################
}

void KAboutDialog::setVersion(const QString& name)
{
  // #################################################################
  if( about == 0 ) { return; }
  about->setVersion(name);
  // #################################################################
}

void KAboutDialog::sendEmailSlot(const QString& name, const QString& email)
{
  kapp->invokeMailer( email, QString::null );
  /*
  debug("KAboutDialog::sendEmailSlot: request to send an email to %s <%s>.",
	name.ascii(), email.ascii());
  emit(sendEmail(name, email));
  */
}

void KAboutDialog::openURLSlot(const QString& url)
{
  kapp->invokeBrowser( url );
  //debug("KAboutDialog::openURLSlot: request to open URL <%s>.", url.ascii());
  //emit(openURL(url));
}


void KAboutDialog::mouseTrackSlot( int mode, const QMouseEvent *e )
{
  // By default we do nothing. This method must be reimplemented.
}


QFrame *KAboutDialog::addTextPage( const QString &title, const QString &text, 
				   bool richText, int numLines )
{
  if( mContainerBase == 0 ) { return( 0 ); }
  return( mContainerBase->addTextPage( title, text, richText, numLines ) );
}


KAboutContainer *KAboutDialog::addContainerPage( const QString &title, 
				  int childAlignment, int innerAlignment ) 
{
  if( mContainerBase == 0 ) { return( 0 ); }
  return( mContainerBase->addContainerPage( title, childAlignment, 
					    innerAlignment) );
}


QFrame *KAboutDialog::addPage( const QString &title )
{
  if( mContainerBase == 0 ) { return( 0 ); }
  return( mContainerBase->addEmptyPage( title ) );
}


KAboutContainer *KAboutDialog::addContainer( int childAlignment,
					     int innerAlignment ) 
{
  if( mContainerBase == 0 ) { return( 0 ); }
  return( mContainerBase->addContainer( childAlignment, innerAlignment ) );
}


void KAboutDialog::setTitle( const QString &title )
{
  if( mContainerBase == 0 ) { return; }
  mContainerBase->setTitle( title );
}


void KAboutDialog::setImage( const char *fileName )
{
  if( mContainerBase == 0 ) { return; }
  mContainerBase->setImage( fileName );
}


void KAboutDialog::setImageBackgroundColor( const QColor &color )
{
  if( mContainerBase == 0 ) { return; }
  mContainerBase->setImageBackgroundColor( color );
}


void KAboutDialog::setImageFrame( bool state )
{
  if( mContainerBase == 0 ) { return; }
  mContainerBase->setImageFrame( state );
}


void KAboutDialog::setProduct( const QString &appName, const QString &version,
			       const QString &author, const QString &year )
{
  if( mContainerBase == 0 ) { return; }
  mContainerBase->setProduct( appName, version, author, year );
}



void KAboutDialog::imageURL( QWidget *parent, const QString &caption,
			     const QString &path, const QColor &imageColor,
			     const QString &url )
{
  KAboutDialog a( AbtImageOnly, "", Close, Close, parent, "image", true );
  a.setPlainCaption( caption );
  a.setImage( path );
  a.setImageBackgroundColor( imageColor );

  KAboutContainer *c = a.addContainer( AlignCenter, AlignCenter ); 
  if( c != 0 )
  {
    c->addPerson( QString::null, QString::null, url, QString::null );
  }
  a.exec();
}




//
// A class that can can monitor mouse movements on the image
//
KImageTrackLabel::KImageTrackLabel( QWidget *p, const char *name, WFlags f )
  : QLabel( p, name, f )
{
  setText( i18n("Image missing"));
}

void KImageTrackLabel::mousePressEvent( QMouseEvent *e )
{
  emit mouseTrack( MousePress, e );
}

void KImageTrackLabel::mouseReleaseEvent( QMouseEvent *e )
{
  emit mouseTrack( MouseRelease, e );
}

void KImageTrackLabel::mouseDoubleClickEvent( QMouseEvent *e )
{
  emit mouseTrack( MouseDoubleClick, e );
}

void KImageTrackLabel::mouseMoveEvent ( QMouseEvent *e )
{
  emit mouseTrack( MouseDoubleClick, e );
}

