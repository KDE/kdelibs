/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999 Espen Sand (espensa@online.no)
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

#include <iostream.h>

#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qsplitter.h>
#include <qwidgetstack.h>

#include <kapp.h>
#include <kdialog.h> // Access to some static members
#include <klocale.h>
#include <kglobal.h>
#include <ktabctl.h>
#include <kseparator.h>

#include "kjanuswidget.h"



KJanusWidget::KJanusWidget( QWidget *parent, const char *name, int face )
  : QWidget( parent, name, 0 ),
    mPageList(0), mTreeNodeList(0), mTitleList(0),mActivePageWidget(0),
    mActivePageIndex(-1),mValid(false),mFace(face)
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );

  if( mFace == TreeList )
  {
    mPageList = new QList<QWidget>;
    if( mPageList == 0 ) { return; }

    mTreeNodeList = new QList<QListViewItem>;
    if( mTreeNodeList == 0 ) { return; }

    mTitleList = new QStringList();
    if( mTitleList == 0 ) { return; }

    QSplitter *splitter = new QSplitter( this );
    if( splitter == 0 ) { return; }
    topLayout->addWidget( splitter, 10 );

    //
    // Tree list
    //
    mTreeList = new QListView( splitter );
    if( mTreeList == 0 ) { return; }

    mTreeList->addColumn( QString("") );
    mTreeList->header()->hide();
    mTreeList->setRootIsDecorated(true);
    mTreeList->setSorting( -1 );
    connect( mTreeList, SIGNAL(selectionChanged()), SLOT(slotShowPage()) );

    //
    // Page area. Title at top with a separator below and a pagestack using
    // all available space at bottom.
    //
    QFrame *p = new QFrame( splitter );
    if( p == 0 ) { return; }
 
    QHBoxLayout *hbox = new QHBoxLayout( p, 0, 0 );
    hbox->addSpacing( KDialog::spacingHint() );
    
    QFrame *page = new QFrame( p );
    if( page == 0 ) { return; }
    hbox->addWidget( page, 10 );

    QVBoxLayout *vbox = new QVBoxLayout( page, 0, KDialog::spacingHint() );
    if( vbox == 0 ) { return; }

    mTitleLabel = new QLabel( QString("Empty page"), page );
    vbox->addWidget( mTitleLabel );
    
    QFont titleFont( mTitleLabel->font() );
    titleFont.setBold( true );
    mTitleLabel->setFont( titleFont );

    mTitleSep = new KSeparator( page );
    mTitleSep->setFrameStyle( QFrame::HLine|QFrame::Plain );
    vbox->addWidget( mTitleSep );

    mPageStack = new QWidgetStack( page );
    if( mPageStack == 0 ) { return; }
    vbox->addWidget( mPageStack, 10 );
    hbox->activate();
    vbox->activate();
  }
  else if( mFace == Tabbed )
  {
    mPageList = new QList<QWidget>;
    if( mPageList == 0 ) { return; }
    
    mTabControl = new KTabCtl( this );
    if( mTabControl == 0 ) { return; }
    topLayout->addWidget( mTabControl, 10 );
  }
  else if( mFace == Swallow )
  {
    mSwallowPage = new QWidget( this );
    if( mSwallowPage == 0 ) { return; }
    topLayout->addWidget( mSwallowPage, 10 );
  }
  else
  {
    mFace = Plain;
    mPlainPage = new QWidget( this );
    if( mPlainPage == 0 ) { return; }
    topLayout->addWidget( mPlainPage, 10 );
  }

  connect(kapp,SIGNAL(kdisplayFontChanged()),SLOT(slotFontChanged()));
  mValid = true;

  setSwallowedWidget(0); // Set default size if 'mFace' is Swallow.
  topLayout->activate();
}


KJanusWidget::~KJanusWidget( void )
{
  delete mPageList;
  delete mTreeNodeList;
  delete mTitleList;
}


bool KJanusWidget::isValid( void ) const
{
  return( mValid );
}


QWidget *KJanusWidget::plainPage( void )
{
  return( mPlainPage );
}


int KJanusWidget::face( void ) const
{
  return( mFace );
}


QFrame *KJanusWidget::addPage( const QString &itemName, const QString &header )
{
  if( mValid == false )
  {
    cout << "addPage:" << "Invalid object" << endl;
    return( 0 );
  }

  if( mFace == Tabbed )
  {
    QFrame *page = new QFrame( mTabControl, "page" );
    if( page == 0 ) { return( 0 ); }

    page->setEnabled(false);
    page->hide();

    mTabControl->addTab( page, itemName );
    mPageList->append(page);

    QVBoxLayout *vbox = new QVBoxLayout( page,KDialog:: spacingHint() );
    QFrame *frame = new QFrame( page, "page" );
    vbox->addWidget( frame, 10 );

    return( frame );
  }
  
  if( mFace == TreeList )
  {
    QFrame *page = new QFrame( this, "page" );
    if( page == 0 ) { return(0); }

    page->setEnabled( false );
    //page->setFocusPolicy( StrongFocus );
    mPageList->append( page );
    mPageStack->addWidget( page, 0 );

    QListViewItem *lastItem = mTreeNodeList->getLast();
    if( lastItem == 0 )
    {
      QListViewItem *item = new QListViewItem( mTreeList, itemName );
      mTreeNodeList->append( item );
    }
    else
    {
      QListViewItem *item = new QListViewItem( mTreeList, lastItem, itemName );
      mTreeNodeList->append( item );
    }

    mTreeList->setMinimumWidth( mTreeList->columnWidth(0) +
      mTreeList->lineWidth()*2 + KDialog::spacingHint() );

    //
    // Make sure the title label is sufficiently wide 
    //
    const QString &title = (header != QString::null ? header : itemName);
    QRect r = mTitleLabel->fontMetrics().boundingRect( title );
    if( mTitleLabel->minimumWidth() < r.width() )
    {
      mTitleLabel->setMinimumWidth( r.width() );
    }
    mTitleList->append( title );
    return( page );
  }

  cout << "addPage:" << "Illegal shape" << endl;
  return( 0 );
}



bool KJanusWidget::setSwallowedWidget( QWidget *widget )
{
  if( mFace != Swallow || mValid == false )
  {
    return( false );
  }

  //
  // Remove current layout and make a new.
  //
  if( mSwallowPage->layout() != 0 )
  {
    delete mSwallowPage->layout();
  }
  QGridLayout *gbox = new QGridLayout( mSwallowPage, 1, 1, 0 );
  if( gbox == 0 ) { return( false ); }

  //
  // Hide old children
  //
  QObjectList *l = (QObjectList*)mSwallowPage->children(); // silence please
  for( uint i=0; i < l->count(); i++ )
  {
    QObject *o = l->at(i);
    if( o->isWidgetType() )
    {
      ((QWidget*)o)->hide();
    }
  }

  //
  // Add new child or make default size
  //
  if( widget == 0 )
  {
    gbox->addRowSpacing(0,100);
    gbox->addColSpacing(0,100);
    mSwallowPage->setMinimumSize(100,100);
  }
  else
  {    
    if( widget->parent() != mSwallowPage )
    {
      widget->recreate( mSwallowPage, 0, QPoint(0,0) );
    }
    gbox->addWidget(widget, 0, 0 );
    mSwallowPage->setMinimumSize( widget->minimumSize() );
  }

  return( true );
}





bool KJanusWidget::slotShowPage( void )
{
  if( mFace != TreeList || mValid == false )
  {
    return( false );
  }

  const QListViewItem *node = mTreeList->selectedItem();
  if( node == 0 ) { return( false ); }

  for( uint i=0; i < mTreeNodeList->count(); i++ )
  {
    if( node == mTreeNodeList->at(i) )
    {
      return( showPage(i) );
    }
  }
  
  return( false );
}


bool KJanusWidget::showPage( int index )
{
  if( mPageList == 0 || mValid == false )
  {
    return( false );
  }
  else
  {
    if( showPage( mPageList->at( index ) ) == true )
    {
      mActivePageIndex = index;
      return( true );
    }
    else
    {
      return( false );
    }
  }
}


bool KJanusWidget::showPage( QWidget *w )
{	
  if( mFace != TreeList && mFace != Tabbed || mValid == false )
  {
    return( false );
  }

  if( w == 0 )
  {
    return( false );
  }

  if( mFace == TreeList )
  {
    mPageStack->raiseWidget( w );
    mActivePageWidget = w;

    int index = mPageList->findRef( w );
    mTitleLabel->setText( *mTitleList->at(index) );
    mTreeList->setSelected( mTreeNodeList->at(index), true );
  }
  else 
  {
    w->raise();
    if( mActivePageWidget != 0 )
    {
      mActivePageWidget->setEnabled( false );
      mActivePageWidget->hide();
    } 

    mActivePageWidget = w;
    mActivePageWidget->setEnabled( true );
    mActivePageWidget->show();
  }
  return( true );	
}


int KJanusWidget::activePageIndex( void ) const
{
  return( mFace != TreeList ? -1 : mActivePageIndex );
}


void KJanusWidget::slotFontChanged( void )
{
  mTitleLabel->setFont( KGlobal::generalFont() );
  QFont titleFont( mTitleLabel->font() );
  titleFont.setBold( true );
  mTitleLabel->setFont( titleFont );
}


void KJanusWidget::setFocus( void )
{
  if( mValid == false ) { return; }
  if( mFace == TreeList )
  {
    mTreeList->setFocus();
  }
  else if( mFace == Tabbed )
  {
    mTabControl->setFocus();
  }
  else if( mFace == Swallow )
  {
    mSwallowPage->setFocus();
  }
  else if( mFace == Plain )
  {
    mPlainPage->setFocus();
  }
}


QSize KJanusWidget::minimumSizeHint( void )
{
  if( mFace == TreeList )
  {
    QSize m( KDialog::spacingHint()+style().splitterWidth(), 
	     KDialog::spacingHint()*2 );
    QSize s1( mTreeList->minimumSize() );
    QSize s2( mTitleLabel->sizeHint() );
    QSize s3( mPageStack->sizeHint() );
    
    return( QSize(
      s1.width() + QMAX( s2.width(), s3.width() ) + m.width(),
      s2.height() + s3.height() + m.height() + 
      mTitleSep->minimumSize().height() ));
  }
  else if( mFace == Tabbed )
  {
    return( mTabControl->sizeHint() );
  }
  else if( mFace == Swallow )
  {
    return( mSwallowPage->minimumSize() );
  }
  else if( mFace == Plain )
  {
    return( mPlainPage->sizeHint() );
  }
}

#include "kjanuswidget.moc"
