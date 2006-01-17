/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2001 Mirko Boehm <mirko@kde.org> and
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */


#include <qclipboard.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qobject.h>
#include <qpainter.h>
#include <qrect.h>
#include <qtabwidget.h>
#include <qtabbar.h>
#include <QScrollArea>
#include <qapplication.h>

#include <ktoolinvocation.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <ktextbrowser.h>
#include <kurllabel.h>
#include <kaboutdialog.h>
#include <kiconloader.h>
#include <kaboutdialog_private.h>
#include <kdebug.h>
#include <ktextedit.h>

//MOC_SKIP_BEGIN
template class QList<KAboutContributor *>;
//MOC_SKIP_END

#define WORKTEXT_IDENTATION 16
#define Grid 3

// ##############################################################
// MOC OUTPUT FILES:
#include "kaboutdialog.moc"
#include "kaboutdialog_private.moc"
// ##############################################################

class KAboutTabWidget : public QTabWidget
{
public:
    KAboutTabWidget( QWidget* parent ) : QTabWidget( parent ) {}
    QSize sizeHint() const {
	return QTabWidget::sizeHint().expandedTo( tabBar()->sizeHint() + QSize(4,4) );
    }
};




KAboutContributor::KAboutContributor( QWidget *_parent,
			              const QString &_name,const QString &_email,
			              const QString &_url, const QString &_work,
			              bool showHeader, bool showFrame,
				      bool showBold )
  : QFrame( _parent ), mShowHeader(showHeader), mShowBold(showBold), d(0)
{
  if ( showFrame )
    setFrameStyle( QFrame::Panel | QFrame::Raised );

  mLabel[0] = new QLabel( this );
  mLabel[1] = new QLabel( this );
  mLabel[2] = new QLabel( this );
  mLabel[3] = new QLabel( this );
  mText[0] = new QLabel( this );
  mText[1] = new KURLLabel( this );
  mText[2] = new KURLLabel( this );
  mText[3] = new QLabel( this );

  setName( _name, i18n("Author"), false );
  setEmail( _email, i18n("Email"), false );
  setURL( _url, i18n("Homepage"), false );
  setWork( _work, i18n("Task"), false );

  KURLLabel *kurl = static_cast<KURLLabel *>(mText[1]);
  kurl->setFloat(true);
  kurl->setUnderline(true);
  connect(kurl, SIGNAL(leftClickedURL(const QString &)),
	  SLOT(emailClickedSlot(const QString &)));

  kurl = static_cast<KURLLabel *>(mText[2]);
  kurl->setFloat(true);
  kurl->setUnderline(true);
  connect(kurl, SIGNAL(leftClickedURL(const QString &)),
	  SLOT(urlClickedSlot(const QString &)));

  mLabel[3]->setAlignment( Qt::AlignTop );

  fontChange( font() );
  updateLayout();
}


void KAboutContributor::setName( const QString &_text, const QString &_header,
				 bool _update )
{
  mLabel[0]->setText(_header);
  mText[0]->setText(_text);
  if( _update ) { updateLayout(); }
}


void KAboutContributor::setEmail( const QString &_text, const QString &_header,
				  bool _update )
{
  mLabel[1]->setText(_header);
  KURLLabel* const kurl = static_cast<KURLLabel *>(mText[1]);
  kurl->setText(_text);
  kurl->setURL(_text);
  if( _update ) { updateLayout(); }
}


void KAboutContributor::setURL( const QString &_text, const QString &_header,
				bool _update )
{
  mLabel[2]->setText(_header);
  KURLLabel* const kurl = static_cast<KURLLabel *>(mText[2]);
  kurl->setText(_text);
  kurl->setURL(_text);
  if( _update ) { updateLayout(); }
}


void KAboutContributor::setWork( const QString &_text, const QString &_header,
				 bool _update )
{
  mLabel[3]->setText(_header);
  mText[3]->setText(_text);
  if( _update ) { updateLayout(); }
}


QString KAboutContributor::getName( void ) const
{
  return mText[0]->text();
}


QString KAboutContributor::getEmail( void ) const
{
  return mText[1]->text();
}


QString KAboutContributor::getURL( void ) const
{
  return mText[2]->text();
}


QString KAboutContributor::getWork( void ) const
{
  return mText[3]->text();
}



void KAboutContributor::updateLayout( void )
{
  delete layout();

  int row = 0;
  if( !mText[0]->text().isEmpty() ) { ++row; }
  if( !mText[1]->text().isEmpty() ) { ++row; }
  if( !mText[2]->text().isEmpty() ) { ++row; }
  if( !mText[3]->text().isEmpty() ) { ++row; }


  QGridLayout *gbox;
  if( row == 0 )
  {
    gbox = new QGridLayout( this, 1, 1, 0 );
    for( int i=0; i<4; ++i )
    {
      mLabel[i]->hide();
      mText[i]->hide();
    }
  }
  else
  {
    if( mText[0]->text().isEmpty() && !mShowHeader )
    {
      gbox = new QGridLayout( this, row, 1, frameWidth()+1, 2 );
    }
    else
    {
      gbox = new QGridLayout( this, row, 2, frameWidth()+1, 2 );
      if( !mShowHeader )
      {
	gbox->addColSpacing( 0, KDialog::spacingHint()*2 );
      }
      gbox->setColStretch( 1, 10 );
    }

    for( int i=0, r=0; i<4; ++i )
    {
      mLabel[i]->setFixedHeight( fontMetrics().lineSpacing() );
      if( i != 3 )
      {
	mText[i]->setFixedHeight( fontMetrics().lineSpacing() );
      }

      if( !mText[i]->text().isEmpty() )
      {
	if( mShowHeader )
	{
	  gbox->addWidget( mLabel[i], r, 0, Qt::AlignLeft );
	  gbox->addWidget( mText[i], r, 1, Qt::AlignLeft  );
	  mLabel[i]->show();
	  mText[i]->show();
	}
	else
	{
	  mLabel[i]->hide();
	  if( !i )
	  {
	    gbox->addMultiCellWidget( mText[i], r, r, 0, 1, Qt::AlignLeft );
	  }
	  else
	  {
	    gbox->addWidget( mText[i], r, 1, Qt::AlignLeft  );
	  }
	  mText[i]->show();
	}
	++r;
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


void KAboutContributor::fontChange( const QFont &/*oldFont*/ )
{
  if( mShowBold )
  {
    QFont f( font() );
    f.setBold( true );
    mText[0]->setFont( f );
  }
  update();
}


QSize KAboutContributor::sizeHint( void ) const
{
  return minimumSizeHint();
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
KAboutContainerBase::KAboutContainerBase( int layoutType, QWidget *_parent )
  : QWidget( _parent ),
    mImageLabel(0), mTitleLabel(0), mIconLabel(0),mVersionLabel(0),
    mAuthorLabel(0), mImageFrame(0),mPageTab(0),mPlainSpace(0),d(0)
{
  mTopLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );
  if( !mTopLayout ) { return; }

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
    mTitleLabel->setAlignment(Qt::AlignCenter);
    mTopLayout->addWidget( mTitleLabel );
    mTopLayout->addSpacing( KDialog::spacingHint() );
  }

  if( layoutType & AbtProduct )
  {
      QWidget* const productArea = new  QWidget( this );
      mTopLayout->addWidget( productArea, 0, QApplication::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft );

      QHBoxLayout* const hbox = new QHBoxLayout(productArea,0,KDialog::spacingHint());
      if( !hbox ) { return; }

      mIconLabel = new QLabel( productArea );
      hbox->addWidget( mIconLabel, 0, Qt::AlignLeft|Qt::AlignHCenter );

      QVBoxLayout* const vbox = new QVBoxLayout();
      if( !vbox ) { return; }
      hbox->addLayout( vbox );

      mVersionLabel = new QLabel( productArea, "version" );
      mAuthorLabel  = new QLabel( productArea, "author" );
      vbox->addWidget( mVersionLabel );
      vbox->addWidget( mAuthorLabel );
      hbox->activate();

      mTopLayout->addSpacing( KDialog::spacingHint() );
  }

  QHBoxLayout* const hbox = new QHBoxLayout();
  if( !hbox ) { return; }
  mTopLayout->addLayout( hbox, 10 );

  if( layoutType & AbtImageLeft )
  {
    QVBoxLayout* vbox = new QVBoxLayout();
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
    mPageTab = new KAboutTabWidget( this );
    if( !mPageTab ) { return; }
    hbox->addWidget( mPageTab, 10 );
  }
  else if( layoutType & AbtImageOnly )
  {
    mImageFrame = new QFrame( this );
    setImageFrame( true );
    hbox->addWidget( mImageFrame, 10 );

    QGridLayout* const gbox = new QGridLayout(mImageFrame, 3, 3, 1, 0 );
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
    if( !mPlainSpace ) { return; }
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
}

QSize KAboutContainerBase::sizeHint( void ) const
{
    return minimumSize().expandedTo( QSize( QWidget::sizeHint().width(), 0 ) );
}

void KAboutContainerBase::fontChange( const QFont &/*oldFont*/ )
{
  if( mTitleLabel )
  {
    QFont f( KGlobalSettings::generalFont() );
    f.setBold( true );
    int fs = f.pointSize();
    if (fs == -1)
       fs = QFontInfo(f).pointSize();
    f.setPointSize( fs+2 ); // Lets not make it too big
    mTitleLabel->setFont(f);
  }

  if( mVersionLabel )
  {
    QFont f( KGlobalSettings::generalFont() );
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
  QFrame *const page = addEmptyPage( title );
  if( !page ) { return 0; }
  if( numLines <= 0 ) { numLines = 10; }

  QVBoxLayout* const vbox = new QVBoxLayout( page, KDialog::spacingHint() );

  if( richText )
  {
    KTextBrowser *const browser = new KTextBrowser( page );
    browser->setObjectName( "browser" );
    browser->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
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
    KTextEdit *const textEdit = new KTextEdit( page );
    textEdit->setObjectName( "text" );
    textEdit->setReadOnly( true );
    textEdit->setMinimumHeight( fontMetrics().lineSpacing()*numLines );
    textEdit->setLineWrapMode( QTextEdit::NoWrap );
    vbox->addWidget( textEdit );
  }

  return page;
}

QFrame *KAboutContainerBase::addLicensePage( const QString &title,
					  const QString &text, int numLines)
{
  QFrame *const page = addEmptyPage( title );
  if( !page ) { return 0; }
  if( numLines <= 0 ) { numLines = 10; }

  QVBoxLayout* const vbox = new QVBoxLayout( page, KDialog::spacingHint() );

  KTextEdit* const textEdit = new KTextEdit( page );
  textEdit->setObjectName( "license" );
  textEdit->setFont( KGlobalSettings::fixedFont() );
  textEdit->setReadOnly( true );
  textEdit->setLineWrapMode( QTextEdit::NoWrap );
  textEdit->setText( text );
  textEdit->setMinimumHeight( fontMetrics().lineSpacing()*numLines );
  vbox->addWidget( textEdit );
  return page;
}


KAboutContainer *KAboutContainerBase::addContainerPage( const QString &title,
							Qt::Alignment childAlignment,
							Qt::Alignment innerAlignment )
{
  if( !mPageTab )
  {
    kdDebug(291) << "addPage: " << "Invalid layout" << endl;
    return 0;
  }

  KAboutContainer* const container = new KAboutContainer( 0,
    KDialog::spacingHint(), KDialog::spacingHint(), childAlignment,
						  innerAlignment );
  container->setObjectName( "container" );
  mPageTab->addTab( container, title );

  connect(container, SIGNAL(urlClick(const QString &)),
	  SLOT(slotUrlClick(const QString &)));
  connect(container, SIGNAL(mailClick(const QString &,const QString &)),
	  SLOT(slotMailClick(const QString &,const QString &)));

  return container;
}


KAboutContainer *KAboutContainerBase::addScrolledContainerPage(
				      const QString &title,
				      Qt::Alignment childAlignment,
				      Qt::Alignment innerAlignment )
{
  if( !mPageTab )
  {
    kdDebug(291) << "addPage: " << "Invalid layout" << endl;
    return 0;
  }

  QFrame *const page = addEmptyPage( title );
  QVBoxLayout* const vbox = new QVBoxLayout( page, KDialog::spacingHint() );
  QScrollArea* const scrollView = new QScrollArea( page );
  vbox->addWidget( scrollView );

  KAboutContainer* const container = new KAboutContainer( scrollView,
    KDialog::spacingHint(), KDialog::spacingHint(), childAlignment,
    innerAlignment );
  container->setObjectName( "container" );
  scrollView->setWidget( container );

  connect(container, SIGNAL(urlClick(const QString &)),
	  SLOT(slotUrlClick(const QString &)));
  connect(container, SIGNAL(mailClick(const QString &,const QString &)),
	  SLOT(slotMailClick(const QString &,const QString &)));

  return container;
}


QFrame *KAboutContainerBase::addEmptyPage( const QString &title )
{
  if( !mPageTab )
  {
    kdDebug(291) << "addPage: " << "Invalid layout" << endl;
    return 0;
  }

  QFrame *const page = new QFrame( 0, title.toLatin1() );
  page->setFrameStyle( QFrame::NoFrame );

  mPageTab->addTab( page, title );
  return page;
}


KAboutContainer *KAboutContainerBase::addContainer( Qt::Alignment childAlignment,
						    Qt::Alignment innerAlignment )
{
  KAboutContainer* const container = new KAboutContainer( this,
    0, KDialog::spacingHint(), childAlignment, innerAlignment );
  container->setObjectName( "container" );
  mTopLayout->addWidget( container, 0, childAlignment );

  connect(container, SIGNAL(urlClick(const QString &)),
	  SLOT(slotUrlClick(const QString &)));
  connect(container, SIGNAL(mailClick(const QString &,const QString &)),
	  SLOT(slotMailClick(const QString &,const QString &)));

  return container;
}



void KAboutContainerBase::setTitle( const QString &title )
{
  if( !mTitleLabel )
  {
    kdDebug(291) << "setTitle: " << "Invalid layout" << endl;
    return;
  }
  mTitleLabel->setText(title);
}


void KAboutContainerBase::setImage( const QString &fileName )
{
  if( !mImageLabel )
  {
    kdDebug(291) << "setImage: " << "Invalid layout" << endl;
    return;
  }
  if( fileName.isNull() )
  {
    return;
  }

  const QPixmap logo( fileName );
  if( !logo.isNull() )
    mImageLabel->setPixmap( logo );

  mImageFrame->layout()->activate();
}

void KAboutContainerBase::setProgramLogo( const QString &fileName )
{
  if( fileName.isNull() )
  {
    return;
  }

  const QPixmap logo( fileName );
  setProgramLogo( logo );
}

void KAboutContainerBase::setProgramLogo( const QPixmap &pixmap )
{
  if( !mIconLabel )
  {
    kdDebug(291) << "setProgramLogo: " << "Invalid layout" << endl;
    return;
  }
  if( !pixmap.isNull() )
  {
    mIconLabel->setPixmap( pixmap );
  }
}

void KAboutContainerBase::setImageBackgroundColor( const QColor &color )
{
  if( mImageFrame )
  {
    mImageFrame->setBackgroundColor( color );
  }
}


void KAboutContainerBase::setImageFrame( bool state )
{
  if( mImageFrame )
  {
    if( state )
    {
      mImageFrame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
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
  if( !mIconLabel )
  {
    kdDebug(291) << "setProduct: " << "Invalid layout" << endl;
    return;
  }

  int size = IconSize(KIcon::Desktop);
  mIconLabel->setPixmap( qApp->windowIcon().pixmap(size,size) );

  const QString msg1 = i18n("%1 %2 (Using KDE %3)").arg(appName).arg(version).
    arg(QLatin1String(KDE_VERSION_STRING));
  const QString msg2 = !year.isEmpty() ? i18n("%1 %2, %3").arg(QChar(0xA9)).arg(year).
    arg(author) : QLatin1String("");

  //if (!year.isEmpty())
  //  msg2 = i18n("%1 %2, %3").arg(QChar(0xA9)).arg(year).arg(author);

  mVersionLabel->setText( msg1 );
  mAuthorLabel->setText( msg2 );
  if( msg2.isEmpty() )
  {
    mAuthorLabel->hide();
  }

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

void KAboutContainerBase::slotMailClick( const QString &_name,
					 const QString &_address )
{
  emit mailClick( _name, _address );
}



KAboutContainer::KAboutContainer( QWidget *_parent,
				  int _margin, int _spacing,
				  Qt::Alignment childAlignment, Qt::Alignment innerAlignment )
  : QFrame( _parent ), d(0)
{
  mAlignment = innerAlignment;

  QGridLayout* const gbox = new QGridLayout( this, 3, 3, _margin, _spacing );
  if( childAlignment & Qt::AlignHCenter )
  {
    gbox->setColStretch( 0, 10 );
    gbox->setColStretch( 2, 10 );
  }
  else if( childAlignment & Qt::AlignRight )
  {
    gbox->setColStretch( 0, 10 );
  }
  else
  {
    gbox->setColStretch( 2, 10 );
  }

  if( childAlignment & Qt::AlignVCenter )
  {
    gbox->setRowStretch( 0, 10 );
    gbox->setRowStretch( 2, 10 );
  }
  else if( childAlignment & Qt::AlignRight )
  {
    gbox->setRowStretch( 0, 10 );
  }
  else
  {
    gbox->setRowStretch( 2, 10 );
  }

  mVbox = new QVBoxLayout( _spacing );
  gbox->addLayout( mVbox, 1, 1 );
  gbox->activate();
}


void KAboutContainer::childEvent( QChildEvent *e )
{
  if( !e->inserted() || !e->child()->isWidgetType() )
  {
    return;
  }

  QWidget* const w = static_cast<QWidget *>(e->child());
  mVbox->addWidget( w, 0, mAlignment );
  const QSize s( sizeHint() );
  setMinimumSize( s );

  const QList<QObject*> l = children(); // silence please
  foreach ( QObject *o, l ) {
	if ( o->isWidgetType() )
	{
        static_cast<QWidget *>(o)->setMinimumWidth( s.width() );
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

  QSize total_size;

  int numChild = 0;
  const QList<QObject*> l = children(); // silence please
  foreach ( QObject *o, l ) {
	  if ( o->isWidgetType() )
	  {
		  ++numChild;
		  QWidget* const w= static_cast<QWidget *>(o);

		  QSize s = w->minimumSize();
		  if( s.isEmpty() )
		  {
			  s = w->minimumSizeHint();
			  if( s.isEmpty() )
			  {
				  s = w->sizeHint();
				  if( s.isEmpty() )
				  {
					  s = QSize( 100, 100 ); // Default size
				  }
			  }
		  }
		  total_size.setHeight( total_size.height() + s.height() );
		  if( s.width() > total_size.width() ) { total_size.setWidth( s.width() ); }
	  }
  }

  if( numChild > 0 )
  {
    //
    // Seems I have to add 1 to the height to properly show the border
    // of the last entry if layout()->margin() is 0
    //

    total_size.setHeight( total_size.height() + layout()->spacing()*(numChild-1) );
    total_size += QSize( layout()->margin()*2, layout()->margin()*2 + 1 );
  }
  else
  {
    total_size = QSize( 1, 1 );
  }
  return total_size;
}


QSize KAboutContainer::minimumSizeHint( void ) const
{
  return sizeHint();
}


void KAboutContainer::addWidget( QWidget *widget )
{
  widget->reparent( this, 0, QPoint(0,0) );
}


void KAboutContainer::addPerson( const QString &_name, const QString &_email,
				 const QString &_url, const QString &_task,
				 bool showHeader, bool showFrame,bool showBold)
{

  KAboutContributor* const cont = new KAboutContributor( this,
    _name, _email, _url, _task, showHeader, showFrame, showBold );
  cont->setObjectName( "pers" );
  connect( cont, SIGNAL( openURL(const QString&)),
	   this, SIGNAL( urlClick(const QString &)));
  connect( cont, SIGNAL( sendEmail(const QString &, const QString &)),
	   this, SIGNAL( mailClick(const QString &, const QString &)));
}


void KAboutContainer::addTitle( const QString &title, Qt::Alignment alignment,
				bool showFrame, bool showBold )
{

  QLabel* const label = new QLabel( title, this, "title" );
  if( showBold  )
  {
    QFont labelFont( font() );
    labelFont.setBold( true );
    label->setFont( labelFont );
  }
  if( showFrame )
    label->setFrameStyle(QFrame::Panel | QFrame::Raised);

  label->setAlignment( alignment );
}


void KAboutContainer::addImage( const QString &fileName, Qt::Alignment alignment )
{
  if( fileName.isNull() )
  {
    return;
  }

  KImageTrackLabel* const label = new KImageTrackLabel( this );
  label->setObjectName( "image" );
  const QImage logo( fileName );
  if( !logo.isNull() )
  {
    QPixmap pix;
    pix = logo;
    label->setPixmap( pix );
  }
  label->setAlignment( alignment );
}

#if 0
//MOC_SKIP_BEGIN

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
      kdDebug() << "KAboutContributor::KAboutContributor: Out of memory." << endl;
      qApp->quit();
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
  kdDebug() << "KAboutContributor::emailClickedSlot: called." << endl;
  emit(sendEmail(name->text(), e));
  // ###########################################################
}

void
KAboutContributor::urlClickedSlot(const QString& u)
{
  // ###########################################################
  kdDebug() << "KAboutContributor::urlClickedSlot: called." << endl;
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
  maxx=qMax(maxx, email->sizeHint().width()+WORKTEXT_IDENTATION);
  // ----- now determine "work" text rectangle:
  if(!work.isEmpty()) // save time
    {
      rect=fontMetrics().boundingRect
	(0, 0, WorkTextWidth, 32000, Qt::TextWordWrap | Qt::AlignLeft, work);
    }
  if(maxx<rect.width())
  {
    maxx=WorkTextWidth+WORKTEXT_IDENTATION;
  }
  maxx=qMax(maxx, url->sizeHint().width()+WORKTEXT_IDENTATION);
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
  paint.drawText(WORKTEXT_IDENTATION, cy, w, h, Qt::AlignLeft | Qt::TextWordWrap, work);
  // ############################################################################
}
// MOC_SKIP_END
#endif


#if 0
QSize KAboutContributor::sizeHint( void )
{
  int s = KDialog::spacingHint();
  int h = fontMetrics().lineSpacing()*3 + 2*s;
  int m = frameWidth();

  int w = name->sizeHint().width();
  w = qMax( w, email->sizeHint().width()+s);
  w = qMax( w, url->sizeHint().width()+s);

  if( work.isEmpty() == false )
  {
    const int WorkTextWidth=200;
    QRect r = fontMetrics().boundingRect
      (0, 0, WorkTextWidth, 32000, Qt::TextWordWrap | Qt::AlignLeft, work);
    if( w < r.width() )
    {
      w = qMax( w, WorkTextWidth+s );
    }
    h += qMax( fontMetrics().lineSpacing(), r.height() ) + s;
  }
  return( QSize( w + 2*m, h + 2*m ) );


  /*
  int s = 3;
  int m = frameWidth() + KDialog::spacingHint();
  int h = ls * 3 + s * 2;
  int w = name->sizeHint().width();

  w = qMax( w, email->sizeHint().width()+WORKTEXT_IDENTATION);
  w = qMax( w, url->sizeHint().width()+WORKTEXT_IDENTATION);
  if( work.isEmpty() == false )
  {
    const int WorkTextWidth=200;

    QRect r = fontMetrics().boundingRect
      (0, 0, WorkTextWidth, 32000, WordBreak | AlignLeft, work);
    if( w < r.width() )
    {
      w = qMax( w, WorkTextWidth + WORKTEXT_IDENTATION );
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
  paint.drawText( x, y, w, h, Qt::AlignLeft | Qt::TextWordWrap, work );

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






KAboutWidget::KAboutWidget(QWidget *_parent)
  : QWidget(_parent),
    version(new QLabel(this)),
    cont(new QLabel(this)),
    logo(new QLabel(this)),
    author(new KAboutContributor(this)),
    maintainer(new KAboutContributor(this)),
    showMaintainer(false),
    d(0)
{
  // #################################################################
  if( !version || !cont || !logo || !author || !maintainer )
  {
    // this will nearly never happen (out of memory in about box?)
    kdDebug() << "KAboutWidget::KAboutWidget: Out of memory." << endl;
    qApp->quit();
  }
  // -----
  cont->setText(i18n("Other Contributors:"));
  logo->setText(i18n("(No logo available)"));
  logo->setFrameStyle(QFrame::Panel | QFrame::Raised);
  version->setAlignment(Qt::AlignCenter);
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
  int maintWidth, maintHeight;
  QSize total_size;
  // -----
  if(showMaintainer)
    {
      total_size=maintainer->sizeHint();
      maintWidth=total_size.width();
      maintHeight=total_size.height();
    } else {
      maintWidth=0;
      maintHeight=0;
    }
  total_size=author->sizeHint();
  logo->adjustSize();
  cy=version->sizeHint().height()+Grid;
  cx=logo->width();
  tempx=qMax(total_size.width(), maintWidth);
  cx+=Grid+tempx;
  cx=qMax(cx, version->sizeHint().width());
  cy+=qMax(logo->height(),
	   total_size.height()+(showMaintainer ? Grid+maintHeight : 0));
  // -----
  if(!contributors.isEmpty())
    {
      cx=qMax(cx, cont->sizeHint().width());
      cy+=cont->sizeHint().height()+Grid;
      foreach(KAboutContributor* currEntry, contributors)
	{
	  cy+=currEntry->sizeHint().height();
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

void KAboutWidget::sendEmailSlot(const QString &_name, const QString &_email)
{
  emit(sendEmail(_name, _email));
}

void KAboutWidget::openURLSlot(const QString& _url)
{
  emit(openURL(_url));
}

void
KAboutWidget::setAuthor(const QString &_name, const QString &_email,
			const QString &_url, const QString &_w)
{
  // ############################################################################
  author->setName(_name);
  author->setEmail(_email);
  author->setURL(_url);
  author->setWork(_w);
  // ############################################################################
}

void
KAboutWidget::setMaintainer(const QString &_name, const QString &_email,
			    const QString &_url, const QString &_w)
{
  // ############################################################################
  maintainer->setName(_name);
  maintainer->setEmail(_email);
  maintainer->setWork(_w);
  maintainer->setURL(_url);
  showMaintainer=true;
  // ############################################################################
}

void
KAboutWidget::addContributor(const QString &_name, const QString &_email,
			     const QString &_url, const QString &_w)
{
  // ############################################################################
  KAboutContributor* const c=new KAboutContributor(this);
  // -----
  c->setName(_name);
  c->setEmail(_email);
  c->setURL(_url);
  c->setWork(_w);
  contributors.append(c);
  connect(c, SIGNAL(sendEmail(const QString&, const QString&)),
	  SLOT(sendEmailSlot(const QString&, const QString&)));
  connect(c, SIGNAL(openURL(const QString&)), SLOT(openURLSlot(const QString&)));
  // ############################################################################
}

void
KAboutWidget::setVersion(const QString &_name)
{
  // ############################################################################
  version->setText(_name);
  // ############################################################################
}

void
KAboutWidget::resizeEvent(QResizeEvent*)
{
  // ############################################################################
  int _x=0, _y, cx, tempx, tempy;
  // ----- set version label geometry:
  version->setGeometry(0, 0, width(), version->sizeHint().height());
  _y=version->height()+Grid;
  // ----- move logo to correct position:
  logo->adjustSize();
  logo->move(0, _y);
  // ----- move author and maintainer right to it:
  tempx=logo->width()+Grid;
  cx=width()-tempx;
  author->setGeometry
    (tempx, _y, cx, author->sizeHint().height());
  maintainer->setGeometry
    (tempx, _y+author->height()+Grid, cx, maintainer->sizeHint().height());

  _y+=qMax(logo->height(),
	  author->height()+(showMaintainer ? Grid+maintainer->height() : 0));
  // -----
  if(!contributors.isEmpty())
    {
      tempy=cont->sizeHint().height();
      cont->setGeometry(0, _y, width(), tempy);
      cont->show();
      _y+=tempy+Grid;
    } else {
      cont->hide();
    }
  foreach(KAboutContributor *currEntry, contributors)
    {
      tempy=currEntry->sizeHint().height();
      // y+=Grid;
      currEntry->setGeometry(_x, _y, width(), tempy);
      _y+=tempy;
    }
  if(showMaintainer)
    {
      maintainer->show();
    } else {
      maintainer->hide();
    }
  // ############################################################################
}

KAboutDialog::KAboutDialog(QWidget *_parent, bool modal)
  : KDialog(_parent, QString(), Ok ),
    about(new KAboutWidget(this)), mContainerBase(0), d(0)
{
  setModal(modal);
  // #################################################################
  if(!about)
  {
    // this will nearly never happen (out of memory in about box?)
    kdDebug() << "KAboutDialog::KAboutDialog: Out of memory." << endl;
    qApp->quit();
  }
  setMainWidget(about);
  connect(about, SIGNAL(sendEmail(const QString&, const QString&)),
	  SLOT(sendEmailSlot(const QString&, const QString&)));
  connect(about, SIGNAL(openURL(const QString&)),
	  SLOT(openURLSlot(const QString&)));
  // #################################################################
}


KAboutDialog::KAboutDialog( int layoutType, const QString &_caption,
							ButtonCodes buttonMask, ButtonCode defaultButton,
			    QWidget *_parent, bool modal,
			    bool separator, const QString &user1,
			    const QString &user2, const QString &user3 )
  :KDialog( _parent, QString(), buttonMask, 0 , user1, user2, user3 ),
   about(0), d(0)
{
  setModal(modal);
  enableButtonSeparator( separator );
  setDefaultButton(defaultButton);
  setPlainCaption( i18n("About %1").arg(_caption) );

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
  if( mContainerBase ) { mContainerBase->show(); }
  QDialog::show();
}


void KAboutDialog::show( QWidget * /*centerParent*/ )
{
  adjust();
  if( mContainerBase ) { mContainerBase->show(); }
  QDialog::show();
}


void KAboutDialog::adjust()
{
  if( !about ) { return; }
  about->adjust();
  //initializeGeometry();
  resize( sizeHint() );
}


void KAboutDialog::setLogo(const QPixmap& i)
{
  if( !about ) { return; }
  about->setLogo(i);
}


void KAboutDialog::setMaintainer(const QString &_name, const QString &_email,
				 const QString &_url, const QString &_w)
{
  // #################################################################
  if( !about ) { return; }
  about->setMaintainer(_name, _email, _url, _w);
  // #################################################################
}

void KAboutDialog::setAuthor(const QString &_name, const QString &_email,
			     const QString &_url, const QString &_work)
{
  // #################################################################
  if( !about ) { return; }
  about->setAuthor(_name, _email, _url, _work);
  // #################################################################
}

void KAboutDialog::addContributor(const QString &_name, const QString &_email,
				  const QString &_url, const QString &_w)
{
  // #################################################################
  if( !about ) { return; }
  about->addContributor(_name, _email, _url, _w);
  // #################################################################
}

void KAboutDialog::setVersion(const QString &_name)
{
  // #################################################################
  if( !about ) { return; }
  about->setVersion(_name);
  // #################################################################
}

void KAboutDialog::sendEmailSlot(const QString& /*name*/, const QString& email)
{
  KToolInvocation::invokeMailer( email, QString() );
  /*
  kdDebug() << "KAboutDialog::sendEmailSlot: request to send an email to "
	<< name << ", " << email << endl;
  emit(sendEmail(name, email));
  */
}

void KAboutDialog::openURLSlot(const QString& url)
{
  KToolInvocation::invokeBrowser( url );
  //kdDebug() << "KAboutDialog::openURLSlot: request to open URL " << url << endl;
  //emit(openURL(url));
}


void KAboutDialog::mouseTrackSlot( int /*mode*/, const QMouseEvent * /*e*/ )
{
  // By default we do nothing. This method must be reimplemented.
}


QFrame *KAboutDialog::addTextPage( const QString &title, const QString &text,
				   bool richText, int numLines )
{
  if( !mContainerBase ) { return 0; }
  return mContainerBase->addTextPage( title, text, richText, numLines );
}

QFrame *KAboutDialog::addLicensePage( const QString &title, const QString &text,
				   int numLines )
{
  if( !mContainerBase ) { return 0; }
  return mContainerBase->addLicensePage( title, text, numLines );
}


KAboutContainer *KAboutDialog::addContainerPage( const QString &title,
				  Qt::Alignment childAlignment, Qt::Alignment innerAlignment )
{
  if( !mContainerBase ) { return 0; }
  return mContainerBase->addContainerPage( title, childAlignment,
					    innerAlignment);
}


KAboutContainer *KAboutDialog::addScrolledContainerPage( const QString &title,
				  Qt::Alignment childAlignment, Qt::Alignment innerAlignment )
{
  if( !mContainerBase ) { return 0; }
  return mContainerBase->addScrolledContainerPage( title, childAlignment,
						    innerAlignment);
}



QFrame *KAboutDialog::addPage( const QString &title )
{
  if( !mContainerBase ) { return 0; }
  return mContainerBase->addEmptyPage( title );
}


KAboutContainer *KAboutDialog::addContainer( Qt::Alignment childAlignment,
					     Qt::Alignment innerAlignment )
{
  if( !mContainerBase ) { return 0; }
  return mContainerBase->addContainer( childAlignment, innerAlignment );
}


void KAboutDialog::setTitle( const QString &title )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setTitle( title );
}


void KAboutDialog::setImage( const QString &fileName )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setImage( fileName );
}

void KAboutDialog::setProgramLogo( const QString &fileName )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setProgramLogo( fileName );
}

void KAboutDialog::setProgramLogo( const QPixmap &pixmap )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setProgramLogo( pixmap );
}

void KAboutDialog::setImageBackgroundColor( const QColor &color )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setImageBackgroundColor( color );
}


void KAboutDialog::setImageFrame( bool state )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setImageFrame( state );
}


void KAboutDialog::setProduct( const QString &appName, const QString &version,
			       const QString &author, const QString &year )
{
  if( !mContainerBase ) { return; }
  mContainerBase->setProduct( appName, version, author, year );
}



void KAboutDialog::imageURL( QWidget *_parent, const QString &_caption,
			     const QString &_path, const QColor &_imageColor,
			     const QString &_url )
{
  KAboutDialog a( AbtImageOnly, QString(), Close, Close, _parent, "image", true );
  a.setPlainCaption( _caption );
  a.setImage( _path );
  a.setImageBackgroundColor( _imageColor );

  KAboutContainer* const c = a.addContainer( Qt::AlignCenter, Qt::AlignCenter );
  if( c )
  {
    c->addPerson( QString(), QString(), _url, QString() );
  }
  a.exec();
}




//
// A class that can can monitor mouse movements on the image
//
KImageTrackLabel::KImageTrackLabel( QWidget *_parent, Qt::WFlags f )
  : QLabel( _parent, f )
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

void KAboutDialog::virtual_hook( int id, void* data )
{ KDialog::virtual_hook( id, data ); }

