/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2000 Espen Sand (espensa@online.no)
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

#include <qbitmap.h>
#include <qgrid.h>
#include <qhbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qpixmap.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qwidgetstack.h>

#include <kapp.h>
#include <kdialog.h> // Access to some static members
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kseparator.h>

#include "kjanuswidget.h"


class IconListItem : public QListBoxItem
{
  public:
    IconListItem( QListBox *listbox, const QPixmap &pixmap,
		   const QString &text );
    virtual int height( const QListBox *lb ) const;
    virtual int width( const QListBox *lb ) const;
    int expandMinimumWidth( int width );

  protected:
    const QPixmap &defaultPixmap();
    void paint( QPainter *painter );

  private:
    QPixmap mPixmap;
    int mMinimumWidth;
};



template class QList<QListViewItem>;

KJanusWidget::KJanusWidget( QWidget *parent, const char *name, int face )
  : QWidget( parent, name, 0 ),
    mValid(false), mPageList(0), mTreeNodeList(0), mIconNodeList(0),
    mTitleList(0), mFace(face), mTitleLabel(0), mActivePageWidget(0),
    mActivePageIndex(-1)
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );

  if( mFace == TreeList || mFace == IconList )
  {
    mPageList = new QList<QWidget>;
    if( mPageList == 0 ) { return; }

    mTitleList = new QStringList();
    if( mTitleList == 0 ) { return; }

    QFrame *page;
    if( mFace == TreeList )
    {
      QSplitter *splitter = new QSplitter( this );
      if( splitter == 0 ) { return; }
      topLayout->addWidget( splitter, 10 );
      mTreeListResizeMode = QSplitter::KeepSize;

      mTreeNodeList = new QList<QListViewItem>;
      if( mTreeNodeList == 0 ) { return; }

      mTreeList = new KListView( splitter );
      if( mTreeList == 0 ) { return; }

      mTreeList->addColumn( QString::fromLatin1("") );
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

      page = new QFrame( p );
      if( page == 0 ) { return; }
      hbox->addWidget( page, 10 );
    }
    else
    {
      mIconNodeList = new QList<QListBoxItem>;
      QHBoxLayout *hbox = new QHBoxLayout( topLayout );
      mIconList = new IconListBox( this );
      mIconList->verticalScrollBar()->installEventFilter( this );
      hbox->addWidget( mIconList );
      connect( mIconList, SIGNAL(selectionChanged()), SLOT(slotShowPage()));
      hbox->addSpacing( KDialog::spacingHint() );
      page = new QFrame( this );
      hbox->addWidget( page, 10 );
    }

    //
    // Rest of page area. Title at top with a separator below and a
    // pagestack using all available space at bottom.
    //

    QVBoxLayout *vbox = new QVBoxLayout( page, 0, KDialog::spacingHint() );
    if( vbox == 0 ) { return; }

    mTitleLabel = new QLabel( QString::fromLatin1("Empty page"), page );
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
  }
  else if( mFace == Tabbed )
  {
    mPageList = new QList<QWidget>;
    if( mPageList == 0 ) { return; }

    mTabControl = new QTabWidget( this );
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
    mPlainPage = new QFrame( this );
    if( mPlainPage == 0 ) { return; }
    topLayout->addWidget( mPlainPage, 10 );
  }

  connect(kapp,SIGNAL(kdisplayFontChanged()),SLOT(slotFontChanged()));
  mValid = true;

  setSwallowedWidget(0); // Set default size if 'mFace' is Swallow.
}


KJanusWidget::~KJanusWidget()
{
  delete mPageList;
  delete mTreeNodeList;
  delete mTitleList;
}


bool KJanusWidget::isValid() const
{
  return( mValid );
}


QFrame *KJanusWidget::plainPage()
{
  return( mPlainPage );
}


int KJanusWidget::face() const
{
  return( mFace );
}



QFrame *KJanusWidget::addPage( const QString &itemName, const QString &header,
			       const QPixmap &pixmap )
{
  if( mValid == false )
  {
    qDebug("addPage: Invalid object");
    return( 0 );
  }

  if( mFace == Tabbed )
  {
    QFrame *page = new QFrame( mTabControl, "pageXXX" );
    page->hide();
    addPageWidget( page, itemName, header, pixmap );

    QVBoxLayout *vbox = new QVBoxLayout( page, KDialog::spacingHint() );
    QFrame *frame = new QFrame( page, "pageYYY" );
    vbox->addWidget( frame, 10 );
    return( frame );
  }
  if( mFace == TreeList || mFace == IconList )
  {
    QFrame *page = new QFrame( this, "page" );
    if( page == 0 ) { return(0); }

    addPageWidget( page, itemName, header, pixmap );
    return( page );
  }


  qDebug( "addPage: Illegal shape" );
  return( 0 );
}






QVBox *KJanusWidget::addVBoxPage( const QString &itemName,
				  const QString &header,
				  const QPixmap &pixmap )
{
  if( mValid == false )
  {
    qDebug("addPage: Invalid object");
    return( 0 );
  }

  if( mFace == Tabbed )
  {
    QVBox *page = new QVBox( mTabControl, "page" );
    page->setSpacing( KDialog::spacingHint() );
    page->hide();

    addPageWidget( page, itemName, header, pixmap );
    return( page );
  }

  if( mFace == TreeList || mFace == IconList )
  {
    QVBox *page = new QVBox( this, "page" );
    page->setSpacing( KDialog::spacingHint() );

    addPageWidget( page, itemName, header, pixmap );
    return( page );
  }

  return( 0 );
}


QHBox *KJanusWidget::addHBoxPage( const QString &itemName,
				  const QString &header,
				  const QPixmap &pixmap )
{
  if( mValid == false )
  {
    qDebug("addPage: Invalid object");
    return( 0 );
  }

  if( mFace == Tabbed )
  {
    QHBox *page = new QHBox( mTabControl, "page" );
    page->setSpacing( KDialog::spacingHint() );
    page->hide();

    addPageWidget( page, itemName, header, pixmap );
    return( page );
  }

  if( mFace == TreeList || mFace == IconList )
  {
    QHBox *page = new QHBox( this, "page" );
    page->setSpacing( KDialog::spacingHint() );

    addPageWidget( page, itemName, header, pixmap );
    return( page );
  }

  return( 0 );
}



QGrid *KJanusWidget::addGridPage( int n, QGrid::Direction dir,
				  const QString &itemName,
				  const QString &header,
				  const QPixmap &pixmap )
{
  if( mValid == false )
  {
    qDebug("addPage: Invalid object");
    return( 0 );
  }

  if( mFace == Tabbed )
  {
    QGrid *page = new QGrid( n, dir, mTabControl, "page" );
    page->setSpacing( KDialog::spacingHint() );
    page->hide();

    addPageWidget( page, itemName, header, pixmap );
    return( page );
  }

  if( mFace == TreeList || mFace == IconList )
  {
    QGrid *page = new QGrid( n, dir, this, "page" );
    page->setSpacing( KDialog::spacingHint() );

    addPageWidget( page, itemName, header, pixmap );
    return( page );
  }

  return( 0 );
}


void KJanusWidget::addPageWidget( QFrame *page, const QString &itemName,
				  const QString &header,const QPixmap &pixmap )
{
  if( mFace == Tabbed )
  {
    mTabControl->addTab( page, itemName );
    mPageList->append(page);
  }
  else if( mFace == TreeList || mFace == IconList )
  {
    mPageList->append( page );
    mPageStack->addWidget( page, 0 );

    if( mFace == TreeList )
    {
      QListViewItem *lastItem = mTreeNodeList->getLast();
      if( lastItem == 0 )
      {
	QListViewItem *item = new QListViewItem( mTreeList, itemName );
	mTreeNodeList->append( item );
      }
      else
      {
	QListViewItem *item = new QListViewItem( mTreeList, lastItem,itemName);
	mTreeNodeList->append( item );
      }

      mTreeList->setMinimumWidth( mTreeList->columnWidth(0) +
        mTreeList->lineWidth()*2 + KDialog::spacingHint() );
    }
    else
    {

      IconListItem *item = new IconListItem( mIconList, pixmap, itemName );
      //
      // 2000-06-01 Espen Sand: If I do this with Qt 2.1.1 all sorts of
      // strange things happen. With Qt <= 2.1 it worked but now I must
      // either specify the listbox in the constructor on the item
      // or as below, not both.
      // mIconList->insertItem( item );
      //
      mIconList->invalidateHeight();
      mIconNodeList->append( item );

      //
      // Make sure all list items have stored the same minimum width. The
      // code is a bit hairy and will only work when the new item has been
      // appended to the list.
      //
      int iw = item->width( mIconList );
      for( item = (IconListItem*)mIconNodeList->first(); item != 0;
	   item = (IconListItem*)mIconNodeList->next() )
      {
	iw = item->expandMinimumWidth( iw );
      }

      //
      // 2000-02-26 Espen Sand
      // Make sure the listbox is wide enough to avoid a horizontal scrollbar.
      // Note that the listbox width is set in the eventfilter as well.
      // I have yet not understood why I have to multiply by 4 instead of 2
      // below.
      //
      iw  += (mIconList->frameWidth()*4);
      int lw = mIconList->minimumWidth();
      if( lw < iw )
      {
	mIconList->setFixedWidth( iw );
      }
    }

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

    if( mTitleList->count() == 1 )
    {
      showPage(0);
    }
  }
  else
  {
    qDebug( "addPage: Illegal shape" );
  }

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
      widget->reparent( mSwallowPage, 0, QPoint(0,0) );
    }
    gbox->addWidget(widget, 0, 0 );
    gbox->activate();
    mSwallowPage->setMinimumSize( widget->minimumSize() );
  }

  return( true );
}





bool KJanusWidget::slotShowPage()
{
  if( mValid == false )
  {
    return( false );
  }

  if( mFace == TreeList )
  {
    const QListViewItem *node = mTreeList->selectedItem();
    if( node == 0 ) { return( false ); }

    for( uint i=0; i < mTreeNodeList->count(); i++ )
    {
      if( node == mTreeNodeList->at(i) )
      {
	return( showPage(i) );
      }
    }
  }
  else if( mFace == IconList )
  {
    const QListBoxItem *node = mIconList->item( mIconList->currentItem() );
    if( node == 0 ) { return( false ); }

    for( uint i=0; i < mIconNodeList->count(); i++ )
    {
      if( node == mIconNodeList->at(i) )
      {
	return( showPage(i) );
      }
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
  if( w == 0 || mValid == false )
  {
    return( false );
  }

  if( mFace == TreeList || mFace == IconList )
  {
    mPageStack->raiseWidget( w );
    mActivePageWidget = w;

    int index = mPageList->findRef( w );
    mTitleLabel->setText( *mTitleList->at(index) );
    if( mFace == TreeList )
    {
      mTreeList->setSelected( mTreeNodeList->at(index), true );
    }
    else
    {
      mIconList->setSelected( mIconNodeList->at(index), true );
      //
      // 2000-02-13 Espen Sand
      // Don't ask me why (because I don't know). If I select a page
      // with the mouse the page is not updated until it receives an
      // event. It seems this event get lost if the mouse is not moved
      // when released. The timer ensures te update
      //
      QTimer::singleShot( 0, mActivePageWidget, SLOT(update()) );
    }
  }
  else if( mFace == Tabbed )
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
  else
  {
    return( false );
  }

  return( true );
}


int KJanusWidget::activePageIndex() const
{
  if( mFace == TreeList || mFace == IconList )
  {
    return( mActivePageIndex );
  }
  else if( mFace == Tabbed )
  {
    QWidget *widget = mTabControl->currentPage();
    return( widget == 0 ? -1 : mPageList->findRef( widget ) );
  }
  else
  {
    return( -1 );
  }
}


int KJanusWidget::pageIndex( QWidget *widget ) const
{
  if( widget == 0 )
  {
    return( -1 );
  }
  else if( mFace == TreeList || mFace == IconList )
  {
    return( mPageList->findRef( widget ) );
  }
  else if( mFace == Tabbed )
  {
    //
    // The user gets the real page widget with addVBoxPage(), addHBoxPage()
    // and addGridPage() but not with addPage() which returns a child of
    // the toplevel page. addPage() returns a QFrame so I check for that.
    //
    if( widget->isA("QFrame") )
    {
      return( mPageList->findRef( widget->parentWidget() ) );
    }
    else
    {
      return( mPageList->findRef( widget ) );
    }
  }
  else
  {
    return( -1 );
  }
}







void KJanusWidget::slotFontChanged()
{
  if( mTitleLabel != 0 )
  {
    mTitleLabel->setFont( KGlobalSettings::generalFont() );
    QFont titleFont( mTitleLabel->font() );
    titleFont.setBold( true );
    mTitleLabel->setFont( titleFont );
  }

  if( mFace == IconList )
  {
    mIconList->invalidateHeight();
  }
}


void KJanusWidget::setFocus()
{
  if( mValid == false ) { return; }
  if( mFace == TreeList )
  {
    mTreeList->setFocus();
  }
  if( mFace == IconList )
  {
    mIconList->setFocus();
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


QSize KJanusWidget::minimumSizeHint() const
{
  if( mFace == TreeList || mFace == IconList )
  {
    QSize s1( KDialog::spacingHint(), KDialog::spacingHint()*2 );
    QSize s2(0,0);
    QSize s3(0,0);
    QSize s4( mPageStack->sizeHint() );

    if( mFace == TreeList )
    {
      s1.rwidth() += style().splitterWidth();
      s2 = mTreeList->minimumSize();
    }
    else
    {
      mIconList->updateMinimumHeight();
      s2 = mIconList->minimumSize();
    }

    if( mTitleLabel->isVisible() == true )
    {
      s3 += mTitleLabel->sizeHint();
      s3.rheight() += mTitleSep->minimumSize().height();
    }

    //
    // Select the tallest item. It has only effect in IconList mode
    //
    int h1 = s1.rheight() + s3.rheight() + s4.height();
    int h2 = QMAX( h1, s2.rheight() );

    return( QSize( s1.width()+s2.width()+QMAX(s3.width(),s4.width()), h2 ) );
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
  else
  {
    return( QSize( 100, 100 ) ); // Should never happen though.
  }

}


QSize KJanusWidget::sizeHint() const
{
  return( minimumSizeHint() );
}


void KJanusWidget::setTreeListAutoResize( bool state )
{
  if( mFace == TreeList )
  {
    mTreeListResizeMode = state == false ?
      QSplitter::KeepSize : QSplitter::Stretch;
    QSplitter *splitter = (QSplitter*)(mTreeList->parentWidget());
    splitter->setResizeMode( mTreeList, mTreeListResizeMode );
  }
}


void KJanusWidget::setIconListAllVisible( bool state )
{
  if( mFace == IconList )
  {
    mIconList->setShowAll( state );
  }
}


void KJanusWidget::showEvent( QShowEvent * )
{
  if( mFace == TreeList )
  {
    QSplitter *splitter = (QSplitter*)(mTreeList->parentWidget());
    splitter->setResizeMode( mTreeList, mTreeListResizeMode );
  }
}


//
// 2000-13-02 Espen Sand
// It should be obvious that this eventfilter must only be
// be installed on the vertical scrollbar of the mIconList.
//
bool KJanusWidget::eventFilter( QObject *o, QEvent *e )
{
  if( e->type() == QEvent::Show )
  {
    IconListItem *item = (IconListItem*)mIconList->item(0);
    if( item != 0 )
    {
      int lw = item->width( mIconList );
      int sw = mIconList->verticalScrollBar()->sizeHint().width();
      mIconList->setFixedWidth( lw+sw+mIconList->frameWidth()*4 );
    }
  }
  else if( e->type() == QEvent::Hide )
  {
    IconListItem *item = (IconListItem*)mIconList->item(0);
    if( item != 0 )
    {
      int lw = item->width( mIconList );
      mIconList->setFixedWidth( lw+mIconList->frameWidth()*4 );
    }
  }
  return QWidget::eventFilter( o, e );
}



//
// Code for the icon list box
//


KJanusWidget::IconListBox::IconListBox( QWidget *parent, const char *name,
					WFlags f )
  :KListBox( parent, name, f ), mShowAll(false), mHeightValid(false)
{
}


void KJanusWidget::IconListBox::updateMinimumHeight()
{
  if( mShowAll == true && mHeightValid == false )
  {
    int h = frameWidth()*2;
    for( QListBoxItem *i = item(0); i != 0; i = i->next() )
    {
      h += i->height( this );
    }
    setMinimumHeight( h );
    mHeightValid = true;
  }
}


void KJanusWidget::IconListBox::invalidateHeight()
{
  mHeightValid = false;
}


void KJanusWidget::IconListBox::setShowAll( bool showAll )
{
  mShowAll = showAll;
  mHeightValid = false;
}



IconListItem::IconListItem( QListBox *listbox, const QPixmap &pixmap,
			    const QString &text )
  : QListBoxItem( listbox )
{
  mPixmap = pixmap;
  if( mPixmap.isNull() == true )
  {
    mPixmap = defaultPixmap();
  }
  setText( text );
  mMinimumWidth = 0;
}


int IconListItem::expandMinimumWidth( int width )
{
  mMinimumWidth = QMAX( mMinimumWidth, width );
  return( mMinimumWidth );
}


const QPixmap &IconListItem::defaultPixmap()
{
  static QPixmap *pix=0;
  if( pix == 0 )
  {
    pix = new QPixmap( 32, 32 );
    QPainter p( pix );
    p.eraseRect( 0, 0, pix->width(), pix->height() );
    p.setPen( Qt::red );
    p.drawRect ( 0, 0, pix->width(), pix->height() );
    p.end();

    QBitmap mask( pix->width(), pix->height(), true );
    mask.fill( Qt::black );
    p.begin( &mask );
    p.setPen( Qt::white );
    p.drawRect ( 0, 0, pix->width(), pix->height() );
    p.end();

    pix->setMask( mask );
  }
  return( *pix );
}


void IconListItem::paint( QPainter *painter )
{
  QFontMetrics fm = painter->fontMetrics();
  int wt = fm.boundingRect(text()).width();
  int wp = mPixmap.width();
  int ht = fm.lineSpacing();
  int hp = mPixmap.height();

  painter->drawPixmap( (mMinimumWidth-wp)/2, 5, mPixmap );
  if( text().isEmpty() == false )
  {
    painter->save();
    QFont f( painter->font() );
    f.setBold( true );
    painter->setFont(f);
    painter->drawText( (mMinimumWidth-wt)/2, hp+ht+5, text() );
    painter->restore();
  }
}

int IconListItem::height( const QListBox *lb ) const
{
  if( text().isEmpty() == true )
  {
    return( mPixmap.height() );
  }
  else
  {
    return( mPixmap.height() + lb->fontMetrics().lineSpacing()+10 );
  }
}


int IconListItem::width( const QListBox *lb ) const
{
  int wt = lb->fontMetrics().boundingRect(text()).width()+10;
  int wp = mPixmap.width() + 10;
  int w  = QMAX( wt, wp );
  return( QMAX( w, mMinimumWidth ) );
}





#include "kjanuswidget.moc"
