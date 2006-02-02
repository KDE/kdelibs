/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2000 Espen Sand (espensa@online.no)
 *  Copyright (C) 2003 Ravikiran Rajagopal (ravi@kde.org)
 *  Copyright (C) 2005 Hamish Rodda (rodda@kde.org)
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
 */

#include "kjanuswidget.h"

#include <qbitmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qpainter.h>
#include <qstyle.h>
#include <QStackedWidget>
#include <QTreeWidgetItem>
#include <QListWidget>
#include <QHeaderView>

#ifdef KDE3_SUPPORT
#include <q3grid.h>
#endif

#include <khbox.h>
#include <kvbox.h>
#include <kapplication.h>
#include <kdialog.h> // Access to some static members
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kseparator.h>
#include <kdebug.h>
#include <klistview.h>
#include <kpushbutton.h>
#include <kguiitem.h>

class IconListBox : public QListWidget
{
  public:
    IconListBox( QWidget *parent=0 );
    void updateMinimumHeight();
    void updateWidth();
    void invalidateHeight();
    void invalidateWidth();
    void setShowAll( bool showAll );

  private:
    bool mShowAll;
    bool mHeightValid;
    bool mWidthValid;
};

class IconListItem : public QListWidgetItem
{
  public:
    IconListItem( QListWidget* parent, const QPixmap &pixmap, const QString &text );
    int expandMinimumWidth( int width );

  protected:
    const QPixmap &defaultPixmap();

  private:
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

KJanusWidget::KJanusWidget( QWidget *parent, int face )
  : QWidget( parent ),
    mValid(false), mFace(face), mTitleLabel(0), mActivePageWidget(0),
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
      mTreeList = new QTreeWidget( d->mListFrame );
      mTreeList->setColumnCount(1);
      mTreeList->header()->hide();
      connect( mTreeList, SIGNAL(itemSelectionChanged()), SLOT(slotShowPage()) );
      connect( mTreeList, SIGNAL(itemClicked(QTreeWidgetItem*)), SLOT(slotItemClicked(QTreeWidgetItem*)));

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
      connect( mIconList, SIGNAL(itemSelectionChanged()), SLOT(slotShowPage()));
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
    vbox->addWidget( mTitleLabel, 0, QApplication::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft );

    QFont titleFont( mTitleLabel->font() );
    titleFont.setBold( true );
    mTitleLabel->setFont( titleFont );

    mTitleSep = new KSeparator( page );
    mTitleSep->setFrameStyle( QFrame::HLine|QFrame::Plain );
    vbox->addWidget( mTitleSep );

    mPageStack = new QStackedWidget( page );
    connect(mPageStack, SIGNAL(currentChanged(int)),
            SLOT(slotCurrentChanged(int)));
    vbox->addWidget( mPageStack, 10 );
  }
  else if( mFace == Tabbed )
  {
    d = new KJanusWidgetPrivate;

    mTabControl = new QTabWidget( this );
    mTabControl->setMargin (KDialog::marginHint());
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

QWidget *KJanusWidget::findParent()
{
  if( mFace == Tabbed ) {
    return mTabControl;
  }
  else if (mFace == TreeList || mFace == IconList) {
    return mPageStack;
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
    kDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  QFrame *page = new QFrame( findParent());
  addPageWidget( page, items, header, pixmap );

  return page;
}

void KJanusWidget::pageGone( QObject *obj )
{
  removePage( static_cast<QWidget*>( obj ) );
}

void KJanusWidget::slotReopen( QTreeWidgetItem * item )
{
  if( item )
    item->treeWidget()->expandItem(item);
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
    kDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  QWidget* parent = findParent();
  KVBox *page = new KVBox(parent);
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
    kDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  KHBox *page = new KHBox(findParent());
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

#ifdef KDE3_SUPPORT
Q3Grid *KJanusWidget::addGridPage( int n, Qt::Orientation dir,
				  const QStringList &items,
				  const QString &header,
				  const QPixmap &pixmap )
{
  if( !mValid )
  {
    kDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  Q3Grid *page = new Q3Grid( n, dir, findParent());
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
#endif

void KJanusWidget::insertTreeListItem(const QStringList &items, const QPixmap &pixmap, QFrame *page)
{
  bool isTop = true;
  QTreeWidgetItem *curTop = 0, *child, *last, *newChild;
  int index = 0;
  QStringList curPath;

  foreach (QString name, items) {
    ++index;

    bool isPath = ( index != items.count() );

    child = last = 0;

    // Search for a child with the current Name, and if it we don't
    // find it, then remember the location of the last child.
    if (isTop) {
      int childIndex = -1;
      while (++childIndex < mTreeList->topLevelItemCount()) {
        last = child = mTreeList->topLevelItem(childIndex);
        if (child->text(0) == name)
          goto foundChild;
      }

    } else {
      int childIndex = -1;
      while (++childIndex < curTop->childCount()) {
        last = child = curTop->child(childIndex);
        if (child->text(0) == name)
          goto foundChild;
      }
    }

    child = 0L;

    foundChild:

    if (!last && !child) {
      // This node didn't have any children at all, lets just insert the
      // new child.
      if (isTop) {
        newChild = new QTreeWidgetItem(mTreeList);
        newChild->setText(0, name);
      } else {
        newChild = new QTreeWidgetItem(curTop);
        newChild->setText(0, name);
      }

    }
    else if (child) {
      // we found the given name in this child.
      if (!isPath) {
        kDebug() << "The element inserted was already in the TreeList box!" << endl;
        return;
      }
      else {
        // Ok we found the folder
        newChild  = child;
      }
    }
    else {
      // the node had some children, but we didn't find the given name
      if (isTop) {
        newChild = new QTreeWidgetItem(mTreeList, last);
        newChild->setText(0, name);
      } else {
        newChild = new QTreeWidgetItem(curTop, last);
        newChild->setText(0, name);
      }
    }

    // Now make the element expandable if it is a path component, and make
    // ready for next loop
    if (isPath) {
      curTop = newChild;
      isTop = false;
      curPath << name;

      QString key = curPath.join("_/_");
      if (mFolderIconMap.contains(key)) {
        QPixmap p = mFolderIconMap[key];
        newChild->setIcon(0,p);
      }
    }
    else {
      if (mShowIconsInTreeList) {
        newChild->setIcon(0, pixmap);
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
      kDebug() << "Invalid QStringList, with zero items" << endl;
      return;
    }

    if( mFace == TreeList )
    {
      insertTreeListItem(items, pixmap, page);
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
    kDebug() << "KJanusWidget::addPageWidget: can only add a page in Tabbed, TreeList or IconList modes" << endl;
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
    QList<QTreeWidgetItem*> selectedItems = mTreeList->selectedItems();
    if (!selectedItems.count())
      return false;

    QWidget *stackItem = mTreeListToPageStack[selectedItems.first()];

    // Make sure to call through the virtual function showPage(int)
    return showPage(d->mPageToInt[stackItem]);
  }
  else if( mFace == IconList )
  {
    QList<QListWidgetItem*> selectedItems = mIconList->selectedItems();
    if (!selectedItems.count())
      return false;

    QWidget *stackItem = mIconListToPageStack[selectedItems.first()];

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
      QMap<QTreeWidgetItem*, QWidget *>::ConstIterator it;
      for (it = mTreeListToPageStack.constBegin(); it != mTreeListToPageStack.constEnd(); ++it) {
        if (it.value() == w) {
          mTreeList->setItemSelected(it.key(), true );
          break;
        }
      }
    }
    else
    {
      QMap<QListWidgetItem*, QWidget *>::ConstIterator it;
      for (it = mIconListToPageStack.constBegin(); it != mIconListToPageStack.constEnd(); ++it) {
        if (it.value() == w) {
          mIconList->setItemSelected( it.key(), true );
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
    QList<QTreeWidgetItem*> selectedItems = mTreeList->selectedItems();
    if (!selectedItems.count())
      return -1;

    QWidget *stackItem = mTreeListToPageStack[selectedItems.first()];
    return d->mPageToInt[stackItem];
  }
  else if (mFace == IconList) {
    QList<QListWidgetItem*> selectedItems = mIconList->selectedItems();
    if (!selectedItems.count())
      return -1;

    QWidget *stackItem = mIconListToPageStack[selectedItems.first()];
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
void KJanusWidget::slotItemClicked(QTreeWidgetItem *it)
{
  if(it && (it->childCount()>0))
    it->treeWidget()->setItemExpanded(it, !it->treeWidget()->isItemExpanded(it));
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
    int h2 = qMax( h1, s2.rheight() );

    return QSize( s1.width()+s2.width()+qMax(s3.width(),s4.width()), h2 );
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

static void expandTree(QTreeWidgetItem* item)
{
  for (int i = 0; i < item->childCount(); ++i) {
    QTreeWidgetItem* child = item->child(i);
    item->treeWidget()->expandItem(child);
    expandTree(child);
  }
}

void KJanusWidget::unfoldTreeList( bool persist )
{
  if( mFace == TreeList )
  {
    if( persist )
      connect( mTreeList, SIGNAL( collapsed( QTreeWidgetItem* ) ), this, SLOT( slotReopen( QTreeWidgetItem* ) ) );
    else
      disconnect( mTreeList, SIGNAL( collapsed( QTreeWidgetItem* ) ), this, SLOT( slotReopen( QTreeWidgetItem* ) ) );

    for (int i = 0; i < mTreeList->topLevelItemCount(); ++i)
      expandTree(mTreeList->topLevelItem(i));
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
    QPushButton * button = new QPushButton( text, d->mListFrame);
    button->setObjectName( "KJanusWidget::buttonBelowList" );
    connect( button, SIGNAL( clicked() ), recv, slot );
  }
}

void KJanusWidget::addButtonBelowList( const KGuiItem & item, QObject * recv, const char * slot )
{
  if( ( mFace == TreeList || mFace == IconList ) && d->mListFrame )
  {
    KPushButton * button = new KPushButton( item, d->mListFrame );
    button->setObjectName( "KJanusWidget::buttonBelowList" );
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
/*

20051107 - Hamish Rodda
Let's try to get away without using this... perhaps we should set the appropriate
sizehint and sizepolicy.

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
}*/



//
// Code for the icon list box
//


IconListBox::IconListBox( QWidget *parent)
  : QListWidget( parent ), mShowAll(false), mHeightValid(false),
   mWidthValid(false)
{
}


void IconListBox::updateMinimumHeight()
{
  if( mShowAll && !mHeightValid )
  {
    int h = frameWidth()*2;
    for (int i = 0; i < count(); ++i)
    {
      h += sizeHintForRow( i );
    }
    setMinimumHeight( h );
    mHeightValid = true;
  }
}


void IconListBox::updateWidth()
{
  if( !mWidthValid )
  {
    int maxWidth = qMax(10, sizeHintForColumn(0));
    /*for (int i = 0; i < count(); ++i)
    {
      // FIXME just gives a size hint for the whole column
      int w = sizeHintForColumn(0);
      maxWidth = qMax( w, maxWidth );
    }*/

    for (int i = 0; i < count(); ++i)
    {
      ((IconListItem *)item(i))->expandMinimumWidth( maxWidth );
    }

    if( verticalScrollBar()->isVisible() )
    {
      maxWidth += verticalScrollBar()->sizeHint().width();
    }

    setFixedWidth( maxWidth + frameWidth()*2 );
    mWidthValid = true;
  }
}


void IconListBox::invalidateHeight()
{
  mHeightValid = false;
}


void IconListBox::invalidateWidth()
{
  mWidthValid = false;
}


void IconListBox::setShowAll( bool showAll )
{
  mShowAll = showAll;
  mHeightValid = false;
}



IconListItem::IconListItem( QListWidget *parent, const QPixmap &pixmap,
                                          const QString &text )
  : QListWidgetItem( parent )
{
  if( pixmap.isNull() )
  {
    setIcon(defaultPixmap());
  }
  else
  {
    setIcon(pixmap);
  }
  setText( text );
  mMinimumWidth = 0;
}


int IconListItem::expandMinimumWidth( int width )
{
  mMinimumWidth = qMax( mMinimumWidth, width );
  return mMinimumWidth;
}


const QPixmap &IconListItem::defaultPixmap()
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
    QMutableMapIterator<QTreeWidgetItem*, QWidget *> it = mTreeListToPageStack;
    while (it.hasNext()) {
      it.next();
      if (it.value() == page)
      {
        delete it.key();
        it.remove();
        mPageStack->removeWidget(page);
        d->mIntToTitle.remove(index);
        d->mPageToInt.remove(page);
        d->mIntToPage.remove(index);
        break;
      }
    }
  }
  else if ( mFace == IconList )
  {
    QMutableMapIterator<QListWidgetItem*, QWidget *> it = mIconListToPageStack;
    while (it.hasNext()) {
      it.next();
      if (it.value() == page)
      {
        delete it.key();
        it.remove();
        mPageStack->removeWidget(page);
        d->mIntToTitle.remove(index);
        d->mPageToInt.remove(page);
        d->mIntToPage.remove(index);
        break;
      }
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
    return QString();
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

void KJanusWidget::slotCurrentChanged( int index )
{
  emit currentPageChanged(mPageStack->widget(index));
}

#include "kjanuswidget.moc"
