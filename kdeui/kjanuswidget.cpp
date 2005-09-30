/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2000 Espen Sand (espensa@online.no)
 *  Copyright (C) 2003 Ravikiran Rajagopal (ravi@kde.org)
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
#include <q3grid.h>
#include <khbox.h>
#include <q3header.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <kvbox.h>
#include <qpainter.h>
#include <qstyle.h>
#include <QStackedWidget>
#include <kapplication.h>
#include <kdialog.h> // Access to some static members
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kseparator.h>
#include <kdebug.h>
#include "kjanuswidget.h"
#include <klistview.h>
#include "kpushbutton.h"
#include "kguiitem.h"

class KJanusWidget::IconListItem : public Q3ListBoxItem
{
  public:
    IconListItem( Q3ListBox *listbox, const QPixmap &pixmap,
		   const QString &text );
    virtual int height( const Q3ListBox *lb ) const;
    virtual int width( const Q3ListBox *lb ) const;
    int expandMinimumWidth( int width );

  protected:
    const QPixmap &defaultPixmap();
    void paint( QPainter *painter );

  private:
    QPixmap mPixmap;
    int mMinimumWidth;
};

class KJanusWidget::KJanusWidgetPrivate
{
public:
  KJanusWidgetPrivate() : mNextPageIndex(0), mListFrame( 0 ) { }

  int mNextPageIndex; // The next page index.

  // Dictionary for multipage modes.
  QMap<int,QWidget*> mIntToPage;
  // Reverse dictionary. Used because showPage() may be performance critical.
  QMap<QWidget*,int> mPageToInt;
  // Dictionary of title string associated with page.
  QMap<int, QString> mIntToTitle;

  QWidget * mListFrame;
  QSplitter * mSplitter;
};

template class Q3PtrList<Q3ListViewItem>;


KJanusWidget::KJanusWidget( QWidget *parent, const char *name, int face )
  : QWidget( parent ),
    mValid(false), mPageList(0),
    mTitleList(0), mFace(face), mTitleLabel(0), mActivePageWidget(0),
    mShowIconsInTreeList(false), d(0)
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );

  if( mFace == TreeList || mFace == IconList )
  {
    d = new KJanusWidgetPrivate;
    d->mSplitter = 0;

    QFrame *page;
    if( mFace == TreeList )
    {
      d->mSplitter = new QSplitter( this );
      topLayout->addWidget( d->mSplitter, 10 );
      mTreeListResizeMode = KeepSize;

      d->mListFrame = new QWidget( d->mSplitter );
      QVBoxLayout *dummy = new QVBoxLayout( d->mListFrame, 0, 0 );
      dummy->setAutoAdd( true );
      mTreeList = new KListView( d->mListFrame );
      mTreeList->addColumn( QString::null );
      mTreeList->header()->hide();
      mTreeList->setRootIsDecorated(true);
      mTreeList->setSorting( -1 );
      connect( mTreeList, SIGNAL(selectionChanged()), SLOT(slotShowPage()) );
      connect( mTreeList, SIGNAL(clicked(Q3ListViewItem *)), SLOT(slotItemClicked(Q3ListViewItem *)));

      //
      // Page area. Title at top with a separator below and a pagestack using
      // all available space at bottom.
      //
      QFrame *p = new QFrame( d->mSplitter );

      QHBoxLayout *hbox = new QHBoxLayout( p, 0, 0 );
      hbox->addSpacing( KDialog::marginHint() );

      page = new QFrame( p );
      hbox->addWidget( page, 10 );
    }
    else
    {
      QHBoxLayout *hbox = new QHBoxLayout( topLayout );
      d->mListFrame = new QWidget( this );
      hbox->addWidget( d->mListFrame );

      ( new QVBoxLayout( d->mListFrame, 0, 0 ) )->setAutoAdd( true );
      mIconList = new IconListBox( d->mListFrame );

      QFont listFont( mIconList->font() );
      listFont.setBold( true );
      mIconList->setFont( listFont );

      mIconList->verticalScrollBar()->installEventFilter( this );
      connect( mIconList, SIGNAL(selectionChanged()), SLOT(slotShowPage()));
      hbox->addSpacing( KDialog::marginHint() );
      page = new QFrame( this );
      hbox->addWidget( page, 10 );
    }

    //
    // Rest of page area. Title at top with a separator below and a
    // pagestack using all available space at bottom.
    //

    QVBoxLayout *vbox = new QVBoxLayout( page, 0, KDialog::spacingHint() );

    mTitleLabel = new QLabel( QLatin1String("Empty Page"), page, "KJanusWidgetTitleLabel" );
    vbox->addWidget( mTitleLabel, 0, QApplication::reverseLayout() ? Qt::AlignRight : Qt::AlignLeft );

    QFont titleFont( mTitleLabel->font() );
    titleFont.setBold( true );
    mTitleLabel->setFont( titleFont );

    mTitleSep = new KSeparator( page );
    mTitleSep->setFrameStyle( QFrame::HLine|QFrame::Plain );
    vbox->addWidget( mTitleSep );

    mPageStack = new QStackedWidget( page );
    connect(mPageStack, SIGNAL(aboutToShow(QWidget *)),
            SIGNAL(aboutToShowPage(QWidget *)));
    vbox->addWidget( mPageStack, 10 );
  }
  else if( mFace == Tabbed )
  {
    d = new KJanusWidgetPrivate;

    mTabControl = new QTabWidget( this );
    mTabControl->setMargin (KDialog::marginHint());
    connect(mTabControl, SIGNAL(currentChanged(QWidget *)),
            SIGNAL(aboutToShowPage(QWidget *)));
    topLayout->addWidget( mTabControl, 10 );
  }
  else if( mFace == Swallow )
  {
    mSwallowPage = new QWidget( this );
    topLayout->addWidget( mSwallowPage, 10 );
  }
  else
  {
    mFace = Plain;
    mPlainPage = new QFrame( this );
    topLayout->addWidget( mPlainPage, 10 );
  }

  if ( kapp )
    connect(kapp,SIGNAL(kdisplayFontChanged()),SLOT(slotFontChanged()));
  mValid = true;

  setSwallowedWidget(0); // Set default size if 'mFace' is Swallow.
}


KJanusWidget::~KJanusWidget()
{
  delete d;
}


bool KJanusWidget::isValid() const
{
  return mValid;
}


QFrame *KJanusWidget::plainPage()
{
  return mPlainPage;
}


int KJanusWidget::face() const
{
  return mFace;
}

QWidget *KJanusWidget::FindParent()
{
  if( mFace == Tabbed ) {
    return mTabControl;
  }
  else {
    return this;
  }
}

QFrame *KJanusWidget::addPage( const QStringList &items, const QString &header,
			       const QPixmap &pixmap )
{
  if( !mValid )
  {
    kdDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  QFrame *page = new QFrame( FindParent());
  addPageWidget( page, items, header, pixmap );

  return page;
}

void KJanusWidget::pageGone( QObject *obj )
{
  removePage( static_cast<QWidget*>( obj ) );
}

void KJanusWidget::slotReopen( Q3ListViewItem * item )
{
  if( item )
    item->setOpen( true );
}

QFrame *KJanusWidget::addPage( const QString &itemName, const QString &header,
			       const QPixmap &pixmap )
{
  QStringList items;
  items << itemName;
  return addPage(items, header, pixmap);
}



KVBox *KJanusWidget::addVBoxPage( const QStringList &items,
				  const QString &header,
				  const QPixmap &pixmap )
{
  if( !mValid )
  {
    kdDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  KVBox *page = new KVBox(FindParent());
  page->setSpacing( KDialog::spacingHint() );
  addPageWidget( page, items, header, pixmap );

  return page;
}

KVBox *KJanusWidget::addVBoxPage( const QString &itemName,
				  const QString &header,
				  const QPixmap &pixmap )
{
  QStringList items;
  items << itemName;
  return addVBoxPage(items, header, pixmap);
}

KHBox *KJanusWidget::addHBoxPage( const QStringList &items,
				  const QString &header,
				  const QPixmap &pixmap )
{
  if( !mValid ) {
    kdDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  KHBox *page = new KHBox(FindParent());
  page->setSpacing( KDialog::spacingHint() );
  addPageWidget( page, items, header, pixmap );

  return page;
}

KHBox *KJanusWidget::addHBoxPage( const QString &itemName,
				  const QString &header,
				  const QPixmap &pixmap )
{
  QStringList items;
  items << itemName;
  return addHBoxPage(items, header, pixmap);
}

Q3Grid *KJanusWidget::addGridPage( int n, Qt::Orientation dir,
				  const QStringList &items,
				  const QString &header,
				  const QPixmap &pixmap )
{
  if( !mValid )
  {
    kdDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  Q3Grid *page = new Q3Grid( n, dir, FindParent());
  page->setSpacing( KDialog::spacingHint() );
  addPageWidget( page, items, header, pixmap );

  return page;
}


Q3Grid *KJanusWidget::addGridPage( int n, Qt::Orientation dir,
				  const QString &itemName,
				  const QString &header,
				  const QPixmap &pixmap )
{
  QStringList items;
  items << itemName;
  return addGridPage(n, dir, items, header, pixmap);
}

void KJanusWidget::InsertTreeListItem(const QStringList &items, const QPixmap &pixmap, QFrame *page)
{
  bool isTop = true;
  Q3ListViewItem *curTop = 0, *child, *last, *newChild;
  int index = 1;
  QStringList curPath;

  for ( QStringList::ConstIterator it = items.begin(); it != items.end(); ++it, index++ ) {
    QString name = (*it);
    bool isPath = ( index != items.count() );

    // Find the first child.
    if (isTop) {
      child = mTreeList->firstChild();
    }
    else {
      child = curTop->firstChild();
    }

    // Now search for a child with the current Name, and if it we doesn't
    // find it, then remember the location of the last child.
    for (last = 0; child && child->text(0) != name ; last = child, child = child->nextSibling());

    if (!last && !child) {
      // This node didn't have any children at all, lets just insert the
      // new child.
      if (isTop)
        newChild = new Q3ListViewItem(mTreeList, name);
      else
        newChild = new Q3ListViewItem(curTop, name);

    }
    else if (child) {
      // we found the given name in this child.
      if (!isPath) {
        kdDebug() << "The element inserted was already in the TreeList box!" << endl;
        return;
      }
      else {
        // Ok we found the folder
        newChild  = child;
      }
    }
    else {
      // the node had some children, but we didn't find the given name
      if (isTop)
        newChild = new Q3ListViewItem(mTreeList, last, name);
      else
        newChild = new Q3ListViewItem(curTop, last, name);
    }

    // Now make the element expandable if it is a path component, and make
    // ready for next loop
    if (isPath) {
      newChild->setExpandable(true);
      curTop = newChild;
      isTop = false;
      curPath << name;

      QString key = curPath.join("_/_");
      if (mFolderIconMap.contains(key)) {
        QPixmap p = mFolderIconMap[key];
        newChild->setPixmap(0,p);
      }
    }
    else {
      if (mShowIconsInTreeList) {
        newChild->setPixmap(0, pixmap);
      }
      mTreeListToPageStack.insert(newChild, page);
    }
  }
}

void KJanusWidget::addPageWidget( QFrame *page, const QStringList &items,
				  const QString &header,const QPixmap &pixmap )
{
  connect(page, SIGNAL(destroyed(QObject*)), SLOT(pageGone(QObject*)));

  if( mFace == Tabbed )
  {
    mTabControl->addTab (page, items.last());
    d->mIntToPage[d->mNextPageIndex] = static_cast<QWidget*>(page);
    d->mPageToInt[static_cast<QWidget*>(page)] = d->mNextPageIndex;
    d->mNextPageIndex++;
  }
  else if( mFace == TreeList || mFace == IconList )
  {
    d->mIntToPage[d->mNextPageIndex] = static_cast<QWidget*>(page);
    d->mPageToInt[static_cast<QWidget*>(page)] = d->mNextPageIndex;
    mPageStack->insertWidget( 0,page );

    if (items.isEmpty()) {
      kdDebug() << "Invalid QStringList, with zero items" << endl;
      return;
    }

    if( mFace == TreeList )
    {
      InsertTreeListItem(items, pixmap, page);
    }
    else // mFace == IconList
    {
      QString itemName = items.last();
      IconListItem *item = new IconListItem( mIconList, pixmap, itemName );
      mIconListToPageStack.insert(item, page);
      mIconList->invalidateHeight();
      mIconList->invalidateWidth();

      if (mIconList->isVisible())
        mIconList->updateWidth();
    }

    //
    // Make sure the title label is sufficiently wide
    //
    QString lastName = items.last();
    const QString &title = (!header.isNull() ? header : lastName);
    QRect r = mTitleLabel->fontMetrics().boundingRect( title );
    if( mTitleLabel->minimumWidth() < r.width() )
    {
      mTitleLabel->setMinimumWidth( r.width() );
    }
    d->mIntToTitle[d->mNextPageIndex] = title;
    if( d->mIntToTitle.count() == 1 )
    {
      showPage(0);
    }
    d->mNextPageIndex++;
  }
  else
  {
    kdDebug() << "KJanusWidget::addPageWidget: can only add a page in Tabbed, TreeList or IconList modes" << endl;
  }

}

void KJanusWidget::setFolderIcon(const QStringList &path, const QPixmap &pixmap)
{
  QString key = path.join("_/_");
  mFolderIconMap.insert(key,pixmap);
}



bool KJanusWidget::setSwallowedWidget( QWidget *widget )
{
  if( mFace != Swallow || !mValid )
  {
    return false;
  }

  //
  // Remove current layout and make a new.
  //
  delete mSwallowPage->layout();

  QGridLayout *gbox = new QGridLayout( mSwallowPage, 1, 1, 0 );

  //
  // Hide old children
  //
  QList<QObject*> l = mSwallowPage->children(); // silence please
  foreach ( QObject* o, l ) {
	  if ( o->isWidgetType() )
	  {
		( ( QWidget* )o )->hide();
	  }
  }
  //
  // Add new child or make default size
  //
  if( !widget )
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

  return true;
}

bool KJanusWidget::slotShowPage()
{
  if( !mValid )
  {
    return false;
  }

  if( mFace == TreeList )
  {
    Q3ListViewItem *node = mTreeList->selectedItem();
    if( !node ) { return false; }

    QWidget *stackItem = mTreeListToPageStack[node];
    // Make sure to call through the virtual function showPage(int)
    return showPage(d->mPageToInt[stackItem]);
  }
  else if( mFace == IconList )
  {
    Q3ListBoxItem *node = mIconList->item( mIconList->currentItem() );
    if( !node ) { return false; }
    QWidget *stackItem = mIconListToPageStack[node];
    // Make sure to call through the virtual function showPage(int)
    return showPage(d->mPageToInt[stackItem]);
  }

  return false;
}


bool KJanusWidget::showPage( int index )
{
  if( !d || !mValid )
  {
    return false;
  }
  else
  {
    return showPage(d->mIntToPage[index]);
  }
}


bool KJanusWidget::showPage( QWidget *w )
{
  if( !w || !mValid )
  {
    return false;
  }

  if( mFace == TreeList || mFace == IconList )
  {
    mPageStack->setCurrentWidget( w );
    mActivePageWidget = w;

    int index = d->mPageToInt[w];
    mTitleLabel->setText( d->mIntToTitle[index] );
    if( mFace == TreeList )
    {
      QMap<Q3ListViewItem *, QWidget *>::Iterator it;
      for (it = mTreeListToPageStack.begin(); it != mTreeListToPageStack.end(); ++it){
        Q3ListViewItem *key = it.key();
        QWidget *val = it.data();
        if (val == w) {
          mTreeList->setSelected(key, true );
          break;
        }
      }
    }
    else
    {
      QMap<Q3ListBoxItem *, QWidget *>::Iterator it;
      for (it = mIconListToPageStack.begin(); it != mIconListToPageStack.end(); ++it){
        Q3ListBoxItem *key = it.key();
        QWidget *val = it.data();
        if (val == w) {
          mIconList->setSelected( key, true );
          break;
        }
      }
    }
  }
  else if( mFace == Tabbed )
  {
    mTabControl->showPage(w);
    mActivePageWidget = w;
  }
  else
  {
    return false;
  }

  return true;
}


int KJanusWidget::activePageIndex() const
{
  if( mFace == TreeList) {
    Q3ListViewItem *node = mTreeList->selectedItem();
    if( !node ) { return -1; }
    QWidget *stackItem = mTreeListToPageStack[node];
    return d->mPageToInt[stackItem];
  }
  else if (mFace == IconList) {
    Q3ListBoxItem *node = mIconList->item( mIconList->currentItem() );
    if( !node ) { return false; }
    QWidget *stackItem = mIconListToPageStack[node];
    return d->mPageToInt[stackItem];
  }
  else if( mFace == Tabbed ) {
    QWidget *widget = mTabControl->currentPage();
    return ( !widget ? -1 : d->mPageToInt[widget] );
  }
  else {
    return -1;
  }
}


int KJanusWidget::pageIndex( QWidget *widget ) const
{
  if( !widget )
  {
    return -1;
  }
  else if( mFace == TreeList || mFace == IconList )
  {
    return d->mPageToInt[widget];
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
      return d->mPageToInt[widget->parentWidget()];
    }
    else
    {
      return d->mPageToInt[widget];
    }
  }
  else
  {
    return -1;
  }
}

void KJanusWidget::slotFontChanged()
{
  if( mTitleLabel )
  {
    mTitleLabel->setFont( KGlobalSettings::generalFont() );
    QFont titleFont( mTitleLabel->font() );
    titleFont.setBold( true );
    mTitleLabel->setFont( titleFont );
  }

  if( mFace == IconList )
  {
    QFont listFont( mIconList->font() );
    listFont.setBold( true );
    mIconList->setFont( listFont );
    mIconList->invalidateHeight();
    mIconList->invalidateWidth();
  }
}

// makes the treelist behave like the list of kcontrol
void KJanusWidget::slotItemClicked(Q3ListViewItem *it)
{
  if(it && (it->childCount()>0))
    it->setOpen(!it->isOpen());
}

void KJanusWidget::setFocus()
{
  if( !mValid ) { return; }
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
      s1.rwidth() += style()->pixelMetric( QStyle::PM_SplitterWidth );
      s2 = mTreeList->minimumSize();
    }
    else
    {
      mIconList->updateMinimumHeight();
      mIconList->updateWidth();
      s2 = mIconList->minimumSize();
    }

    if( mTitleLabel->isVisible() )
    {
      s3 += mTitleLabel->sizeHint();
      s3.rheight() += mTitleSep->minimumSize().height();
    }

    //
    // Select the tallest item. It has only effect in IconList mode
    //
    int h1 = s1.rheight() + s3.rheight() + s4.height();
    int h2 = QMAX( h1, s2.rheight() );

    return QSize( s1.width()+s2.width()+QMAX(s3.width(),s4.width()), h2 );
  }
  else if( mFace == Tabbed )
  {
    return mTabControl->sizeHint();
  }
  else if( mFace == Swallow )
  {
    return mSwallowPage->minimumSize();
  }
  else if( mFace == Plain )
  {
    return mPlainPage->sizeHint();
  }
  else
  {
    return QSize( 100, 100 ); // Should never happen though.
  }

}


QSize KJanusWidget::sizeHint() const
{
  return minimumSizeHint();
}


void KJanusWidget::setTreeListAutoResize( bool state )
{
  if( mFace == TreeList )
  {
    mTreeListResizeMode = !state ? KeepSize : Stretch;
    // the splitter's first widget is d->mListFrame
    if( d->mSplitter && d->mSplitter->count() > 0 )
        d->mSplitter->setStretchFactor( 0, mTreeListResizeMode == KeepSize ? 0 : 1 );
  }
}


void KJanusWidget::setIconListAllVisible( bool state )
{
  if( mFace == IconList )
  {
    mIconList->setShowAll( state );
  }
}

void KJanusWidget::setShowIconsInTreeList( bool state )
{
  mShowIconsInTreeList = state;
}

void KJanusWidget::setRootIsDecorated( bool state )
{
  if( mFace == TreeList ) {
    mTreeList->setRootIsDecorated(state);
  }
}

void KJanusWidget::unfoldTreeList( bool persist )
{
  if( mFace == TreeList )
  {
    if( persist )
      connect( mTreeList, SIGNAL( collapsed( Q3ListViewItem * ) ), this, SLOT( slotReopen( Q3ListViewItem * ) ) );
    else
      disconnect( mTreeList, SIGNAL( collapsed( Q3ListViewItem * ) ), this, SLOT( slotReopen( Q3ListViewItem * ) ) );

    for( Q3ListViewItem * item = mTreeList->firstChild(); item; item = item->itemBelow() )
      item->setOpen( true );
  }
}

void KJanusWidget::addWidgetBelowList( QWidget * widget )
{
  if( ( mFace == TreeList || mFace == IconList ) && d->mListFrame )
  {
    widget->reparent( d->mListFrame, QPoint() );
  }
}

void KJanusWidget::addButtonBelowList( const QString & text, QObject * recv, const char * slot )
{
  if( ( mFace == TreeList || mFace == IconList ) && d->mListFrame )
  {
    QPushButton * button = new QPushButton( text, d->mListFrame, "KJanusWidget::buttonBelowList" );
    connect( button, SIGNAL( clicked() ), recv, slot );
  }
}

void KJanusWidget::addButtonBelowList( const KGuiItem & item, QObject * recv, const char * slot )
{
  if( ( mFace == TreeList || mFace == IconList ) && d->mListFrame )
  {
    KPushButton * button = new KPushButton( item, d->mListFrame, "KJanusWidget::buttonBelowList" );
    connect( button, SIGNAL( clicked() ), recv, slot );
  }
}

void KJanusWidget::showEvent( QShowEvent * )
{
  if( mFace == TreeList )
  {
    if( d->mSplitter && d->mSplitter->count() > 0 )
        d->mSplitter->setStretchFactor( 0, mTreeListResizeMode == KeepSize ? 0 : 1 );
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
    if( item )
    {
      int lw = item->width( mIconList );
      int sw = mIconList->verticalScrollBar()->sizeHint().width();
      mIconList->setFixedWidth( lw+sw+mIconList->frameWidth()*2 );
    }
  }
  else if( e->type() == QEvent::Hide )
  {
    IconListItem *item = (IconListItem*)mIconList->item(0);
    if( item )
    {
      int lw = item->width( mIconList );
      mIconList->setFixedWidth( lw+mIconList->frameWidth()*2 );
    }
  }
  return QWidget::eventFilter( o, e );
}



//
// Code for the icon list box
//


KJanusWidget::IconListBox::IconListBox( QWidget *parent, const char *name,
					Qt::WFlags f )
  :KListBox( parent, name, f ), mShowAll(false), mHeightValid(false),
   mWidthValid(false)
{
}


void KJanusWidget::IconListBox::updateMinimumHeight()
{
  if( mShowAll && !mHeightValid )
  {
    int h = frameWidth()*2;
    for( Q3ListBoxItem *i = item(0); i; i = i->next() )
    {
      h += i->height( this );
    }
    setMinimumHeight( h );
    mHeightValid = true;
  }
}


void KJanusWidget::IconListBox::updateWidth()
{
  if( !mWidthValid )
  {
    int maxWidth = 10;
    for( Q3ListBoxItem *i = item(0); i; i = i->next() )
    {
      int w = ((IconListItem *)i)->width(this);
      maxWidth = QMAX( w, maxWidth );
    }

    for( Q3ListBoxItem *i = item(0); i; i = i->next() )
    {
      ((IconListItem *)i)->expandMinimumWidth( maxWidth );
    }

    if( verticalScrollBar()->isVisible() )
    {
      maxWidth += verticalScrollBar()->sizeHint().width();
    }

    setFixedWidth( maxWidth + frameWidth()*2 );
    mWidthValid = true;
  }
}


void KJanusWidget::IconListBox::invalidateHeight()
{
  mHeightValid = false;
}


void KJanusWidget::IconListBox::invalidateWidth()
{
  mWidthValid = false;
}


void KJanusWidget::IconListBox::setShowAll( bool showAll )
{
  mShowAll = showAll;
  mHeightValid = false;
}



KJanusWidget::IconListItem::IconListItem( Q3ListBox *listbox, const QPixmap &pixmap,
                                          const QString &text )
  : Q3ListBoxItem( listbox )
{
  mPixmap = pixmap;
  if( mPixmap.isNull() )
  {
    mPixmap = defaultPixmap();
  }
  setText( text );
  mMinimumWidth = 0;
}


int KJanusWidget::IconListItem::expandMinimumWidth( int width )
{
  mMinimumWidth = QMAX( mMinimumWidth, width );
  return mMinimumWidth;
}


const QPixmap &KJanusWidget::IconListItem::defaultPixmap()
{
  static QPixmap *pix=0;
  if( !pix )
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
  return *pix;
}


void KJanusWidget::IconListItem::paint( QPainter *painter )
{
  QFontMetrics fm = painter->fontMetrics();
  int ht = fm.boundingRect( 0, 0, 0, 0, Qt::AlignCenter, text() ).height();
  int wp = mPixmap.width();
  int hp = mPixmap.height();

  painter->drawPixmap( (mMinimumWidth-wp)/2, 5, mPixmap );
  if( !text().isEmpty() )
  {
    painter->drawText( 0, hp+7, mMinimumWidth, ht, Qt::AlignCenter, text() );
  }
}

int KJanusWidget::IconListItem::height( const Q3ListBox *lb ) const
{
  if( text().isEmpty() )
  {
    return mPixmap.height();
  }
  else
  {
    int ht = lb->fontMetrics().boundingRect( 0, 0, 0, 0, Qt::AlignCenter, text() ).height();
    return (mPixmap.height() + ht + 10);
  }
}


int KJanusWidget::IconListItem::width( const Q3ListBox *lb ) const
{
  int wt = lb->fontMetrics().boundingRect( 0, 0, 0, 0, Qt::AlignCenter, text() ).width() + 10;
  int wp = mPixmap.width() + 10;
  int w  = QMAX( wt, wp );
  return QMAX( w, mMinimumWidth );
}


void KJanusWidget::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

// TODO: In TreeList, if the last child of a node is removed, and there is no corrsponding widget for that node, allow the caller to
// delete the node.
void KJanusWidget::removePage( QWidget *page )
{
  if (!d || !d->mPageToInt.contains(page))
    return;

  int index = d->mPageToInt[page];

  if ( mFace == TreeList )
  {
    QMap<Q3ListViewItem*, QWidget *>::Iterator i;
    for( i = mTreeListToPageStack.begin(); i != mTreeListToPageStack.end(); ++i )
      if (i.data()==page)
      {
        delete i.key();
        mPageStack->removeWidget(page);
        mTreeListToPageStack.remove(i);
        d->mIntToTitle.remove(index);
        d->mPageToInt.remove(page);
        d->mIntToPage.remove(index);
        break;
      }
  }
  else if ( mFace == IconList )
  {
    QMap<Q3ListBoxItem*, QWidget *>::Iterator i;
    for( i = mIconListToPageStack.begin(); i != mIconListToPageStack.end(); ++i )
      if (i.data()==page)
      {
        delete i.key();
        mPageStack->removeWidget(page);
        mIconListToPageStack.remove(i);
        d->mIntToTitle.remove(index);
        d->mPageToInt.remove(page);
        d->mIntToPage.remove(index);
        break;
      }
  }
  else // Tabbed
  {
    mTabControl->removePage(page);
    d->mPageToInt.remove(page);
    d->mIntToPage.remove(index);
  }
}

QString KJanusWidget::pageTitle(int index) const
{
  if (!d || !d->mIntToTitle.contains(index))
    return QString::null;
  else
    return d->mIntToTitle[index];
}

QWidget *KJanusWidget::pageWidget(int index) const
{
  if (!d || !d->mIntToPage.contains(index))
    return 0;
  else
    return d->mIntToPage[index];
}

#include "kjanuswidget.moc"
