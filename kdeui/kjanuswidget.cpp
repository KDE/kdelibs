/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2000 Espen Sand (espensa@online.no)
 *  Copyright (C) 2003 Ravikiran Rajagopal (ravi@kde.org)
 *  Copyright (C) 2005-2006 Hamish Rodda (rodda@kde.org)
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
#include <QTreeWidgetItemIterator>

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
#include <k3listview.h>
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

class KJanusWidgetPrivate
{
public:
  KJanusWidgetPrivate()
    : nextPageIndex(0)
    , listFrame( 0 )
    , valid(false)
    , titleLabel(0)
    , activePageWidget(0)
    , showIconsInTreeList(false)
  {
  }

  int nextPageIndex; // The next page index.

  // Dictionary for multipage modes.
  QMap<int,QWidget*> intToPage;
  // Reverse dictionary. Used because showPage() may be performance critical.
  QMap<QWidget*,int> pageToInt;
  // Dictionary of title string associated with page.
  QMap<int, QString> intToTitle;

  QWidget * listFrame;
  QSplitter * splitter;

  bool         valid;

  int          face;
  QTreeWidget  *treeList;
  IconListBox  *iconList;
  QStackedWidget *pageStack;
  QLabel       *titleLabel;
  QTabWidget   *tabControl;
  QFrame       *plainPage;
  QWidget      *swallowPage;
  QWidget      *activePageWidget;
  KSeparator   *titleSep;
  enum { KeepSize, Stretch } treeListResizeMode;
  bool         showIconsInTreeList;
  QMap<QTreeWidgetItem*, QWidget *> treeListToPageStack;
  QMap<QListWidgetItem*, QWidget *> iconListToPageStack;
  QMap<QString, QPixmap> folderIconMap;
  QMap<QString, QStringList> childrenNames;
  QMap<QString, QWidget *> childPages;
};

KJanusWidget::KJanusWidget( QWidget *parent, int face )
  : QWidget( parent )
  , d(new KJanusWidgetPrivate())
{
  d->face = face;

  QVBoxLayout *topLayout = new QVBoxLayout( this );

  if( d->face == TreeList || d->face == IconList )
  {
    d->splitter = 0;

    QFrame *page;
    if( d->face == TreeList )
    {
      d->splitter = new QSplitter( this );
      topLayout->addWidget( d->splitter );
      d->treeListResizeMode = KJanusWidgetPrivate::KeepSize;

      d->listFrame = new QWidget( d->splitter );
      //d->listFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

      d->treeList = new QTreeWidget( d->listFrame );
      d->treeList->setColumnCount(1);
      d->treeList->header()->hide();
      d->treeList->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
      kDebug() << k_funcinfo << d->treeList->sizeHint() << endl;
      d->treeList->setMinimumSize(d->treeList->sizeHint());
      d->treeList->setSelectionMode(QAbstractItemView::SingleSelection);
      connect( d->treeList, SIGNAL(itemSelectionChanged()), SLOT(slotShowPage()) );
      connect( d->treeList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(slotItemClicked(QTreeWidgetItem*)));

      QVBoxLayout *dummy = new QVBoxLayout(d->listFrame);
      dummy->setMargin( 0 );
      dummy->setSpacing( 0 );
      dummy->addWidget(d->treeList);

      //
      // Page area. Title at top with a separator below and a pagestack using
      // all available space at bottom.
      //
      QFrame *p = new QFrame( d->splitter );
      //p->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

      QHBoxLayout *hbox = new QHBoxLayout( p );
      hbox->setMargin( 0 );
      hbox->setSpacing( 0 );
      hbox->addSpacing( KDialog::marginHint() );

      page = new QFrame( p );
      hbox->addWidget( page );

      d->splitter->addWidget(d->treeList);
      d->splitter->addWidget(p);
    }
    else
    {
      QHBoxLayout *hbox = new QHBoxLayout();
      topLayout->addLayout( hbox );
      d->listFrame = new QWidget( this );
      hbox->addWidget( d->listFrame );

      QLayout *layout = new QVBoxLayout( d->listFrame );
      layout->setMargin(0);
      layout->setSpacing(0);
      d->iconList = new IconListBox( d->listFrame );
      layout->addWidget(d->iconList);

      QFont listFont( d->iconList->font() );
      listFont.setBold( true );
      d->iconList->setFont( listFont );

      d->iconList->verticalScrollBar()->installEventFilter( this );
      connect( d->iconList, SIGNAL(itemSelectionChanged()), SLOT(slotShowPage()));
      hbox->addSpacing( KDialog::marginHint() );
      page = new QFrame( this );
      hbox->addWidget( page );
    }

    //
    // Rest of page area. Title at top with a separator below and a
    // pagestack using all available space at bottom.
    //

    QVBoxLayout *vbox = new QVBoxLayout( page );
    vbox->setMargin( 0 );
    vbox->setSpacing( KDialog::spacingHint() );

    d->titleLabel = new QLabel( i18n("Empty Page"), page );
    vbox->addWidget( d->titleLabel, 0, QApplication::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft );

    QFont titleFont( d->titleLabel->font() );
    titleFont.setBold( true );
    d->titleLabel->setFont( titleFont );

    d->titleSep = new KSeparator( page );
    d->titleSep->setFrameStyle( QFrame::HLine|QFrame::Plain );
    vbox->addWidget( d->titleSep );

    d->pageStack = new QStackedWidget( page );
    connect(d->pageStack, SIGNAL(currentChanged(int)), SLOT(slotCurrentChanged(int)));
    vbox->addWidget( d->pageStack );
  }
  else if( d->face == Tabbed )
  {
    d->tabControl = new QTabWidget( this );
    topLayout->addWidget( d->tabControl );
  }
  else if( d->face == Swallow )
  {
    d->swallowPage = new QWidget( this );
    topLayout->addWidget( d->swallowPage );
  }
  else
  {
    d->face = Plain;
    d->plainPage = new QFrame( this );
    topLayout->addWidget( d->plainPage );
  }

  if ( kapp )
    connect(kapp,SIGNAL(kdisplayFontChanged()),SLOT(slotFontChanged()));
  d->valid = true;

  setSwallowedWidget(0); // Set default size if 'd->face' is Swallow.
}


KJanusWidget::~KJanusWidget()
{
  delete d;
}


bool KJanusWidget::isValid() const
{
  return d->valid;
}


QFrame *KJanusWidget::plainPage()
{
  return d->plainPage;
}


int KJanusWidget::face() const
{
  return d->face;
}

QFrame *KJanusWidget::addPage( const QStringList &items, const QString &header,
                               const QPixmap &pixmap )
{
  if( !d->valid )
  {
    kDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  QFrame *page = new QFrame;
  addPageWidget( page, items, header, pixmap );

  return page;
}

void KJanusWidget::pageGone( QObject *obj )
{
  removePage( static_cast<QWidget*>( obj ) );
}

QFrame *KJanusWidget::addPage( const QString &itemName, const QString &header, const QPixmap &pixmap )
{
  QStringList items;
  items << itemName;
  return addPage(items, header, pixmap);
}



KVBox *KJanusWidget::addVBoxPage( const QStringList &items,
                                  const QString &header,
                                  const QPixmap &pixmap )
{
  if( !d->valid )
  {
    kDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  KVBox *page = new KVBox;
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
  if( !d->valid ) {
    kDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  KHBox *page = new KHBox;
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
  if( !d->valid )
  {
    kDebug() << "addPage: Invalid object" << endl;
    return 0;
  }

  Q3Grid *page = new Q3Grid( n, dir );
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
      while (++childIndex < d->treeList->topLevelItemCount()) {
        last = child = d->treeList->topLevelItem(childIndex);
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
        newChild = new QTreeWidgetItem(d->treeList);
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
        newChild = new QTreeWidgetItem(d->treeList, last);
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
      if (d->folderIconMap.contains(key)) {
        QPixmap p = d->folderIconMap[key];
        newChild->setIcon(0,p);
      }
    }
    else {
      if (d->showIconsInTreeList) {
        newChild->setIcon(0, pixmap);
      }
      d->treeListToPageStack.insert(newChild, page);
    }
  }

  kDebug() << k_funcinfo << d->treeList->sizeHint() << endl;
  layout()->activate();
}

void KJanusWidget::addPageWidget( QFrame *page, const QStringList &items,
                                  const QString &header,const QPixmap &pixmap )
{
  connect(page, SIGNAL(destroyed(QObject*)), SLOT(pageGone(QObject*)));

  if( d->face == Tabbed )
  {
    d->tabControl->addTab (page, items.last());
    d->intToPage[d->nextPageIndex] = static_cast<QWidget*>(page);
    d->pageToInt[static_cast<QWidget*>(page)] = d->nextPageIndex;
    d->nextPageIndex++;
  }
  else if( d->face == TreeList || d->face == IconList )
  {
    d->intToPage[d->nextPageIndex] = static_cast<QWidget*>(page);
    d->pageToInt[static_cast<QWidget*>(page)] = d->nextPageIndex;
    d->pageStack->insertWidget( 0,page );

    if (items.isEmpty()) {
      kDebug() << "Invalid QStringList, with zero items" << endl;
      return;
    }

    if( d->face == TreeList )
    {
      insertTreeListItem(items, pixmap, page);
    }
    else // d->face == IconList
    {
      QString itemName = items.last();
      IconListItem *item = new IconListItem( d->iconList, pixmap, itemName );
      d->iconListToPageStack.insert(item, page);
      d->iconList->invalidateHeight();
      d->iconList->invalidateWidth();

      if (d->iconList->isVisible())
        d->iconList->updateWidth();
    }

    //
    // Make sure the title label is sufficiently wide
    //
    QString lastName = items.last();
    const QString &title = (!header.isNull() ? header : lastName);
    QRect r = d->titleLabel->fontMetrics().boundingRect( title );
    if( d->titleLabel->minimumWidth() < r.width() )
    {
      d->titleLabel->setMinimumWidth( r.width() );
    }
    d->intToTitle[d->nextPageIndex] = title;
    if( d->intToTitle.count() == 1 )
    {
      showPage(0);
    }
    d->nextPageIndex++;
  }
  else
  {
    kDebug() << "KJanusWidget::addPageWidget: can only add a page in Tabbed, TreeList or IconList modes" << endl;
  }

}

void KJanusWidget::setFolderIcon(const QStringList &path, const QPixmap &pixmap)
{
  QString key = path.join("_/_");
  d->folderIconMap.insert(key,pixmap);
}



bool KJanusWidget::setSwallowedWidget( QWidget *widget )
{
  if( d->face != Swallow || !d->valid )
  {
    return false;
  }

  //
  // Remove current layout and make a new.
  //
  delete d->swallowPage->layout();

  QGridLayout *gbox = new QGridLayout( d->swallowPage );
  gbox->setMargin(0);

  //
  // Hide old children
  //
  QList<QObject*> l = d->swallowPage->children(); // silence please
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
    gbox->addItem(new QSpacerItem(0,100), 0, 0);
    gbox->addItem(new QSpacerItem(100,0), 0, 0);
    d->swallowPage->setMinimumSize(100,100);
  }
  else
  {
    if( widget->parent() != d->swallowPage )
    {
      widget->setParent( d->swallowPage );
    }
    gbox->addWidget(widget, 0, 0 );
    gbox->activate();
    d->swallowPage->setMinimumSize( widget->minimumSize() );
  }

  return true;
}

bool KJanusWidget::slotShowPage()
{
  if( !d->valid )
  {
    return false;
  }

  if( d->face == TreeList )
  {
    QList<QTreeWidgetItem*> selectedItems = d->treeList->selectedItems();
    if (!selectedItems.count())
      return false;

    QWidget *stackItem = d->treeListToPageStack[selectedItems.first()];

    // Make sure to call through the virtual function showPage(int)
    return showPage(d->pageToInt[stackItem]);
  }
  else if( d->face == IconList )
  {
    QList<QListWidgetItem*> selectedItems = d->iconList->selectedItems();
    if (!selectedItems.count())
      return false;

    QWidget *stackItem = d->iconListToPageStack[selectedItems.first()];

    // Make sure to call through the virtual function showPage(int)
    return showPage(d->pageToInt[stackItem]);
  }

  return false;
}

bool KJanusWidget::showPage( int index )
{
  if( !d || !d->valid )
  {
    return false;
  }
  else
  {
    return showPage(d->intToPage[index]);
  }
}


bool KJanusWidget::showPage( QWidget *w )
{
  if( !w || !d->valid )
  {
    return false;
  }

  if( d->face == TreeList || d->face == IconList )
  {
    d->pageStack->setCurrentWidget( w );
    d->activePageWidget = w;

    int index = d->pageToInt[w];
    d->titleLabel->setText( d->intToTitle[index] );
    if( d->face == TreeList )
    {
      QMap<QTreeWidgetItem*, QWidget *>::ConstIterator it;
      for (it = d->treeListToPageStack.constBegin(); it != d->treeListToPageStack.constEnd(); ++it) {
        if (it.value() == w) {
          d->treeList->setItemSelected(it.key(), true );
          break;
        }
      }
    }
    else
    {
      QMap<QListWidgetItem*, QWidget *>::ConstIterator it;
      for (it = d->iconListToPageStack.constBegin(); it != d->iconListToPageStack.constEnd(); ++it) {
        if (it.value() == w) {
          d->iconList->setItemSelected( it.key(), true );
          break;
        }
      }
    }
  }
  else if( d->face == Tabbed )
  {
    d->tabControl->setCurrentIndex(d->tabControl->indexOf(w));
    d->activePageWidget = w;
  }
  else
  {
    return false;
  }

  return true;
}


int KJanusWidget::activePageIndex() const
{
  if( d->face == TreeList) {
    QList<QTreeWidgetItem*> selectedItems = d->treeList->selectedItems();
    if (!selectedItems.count())
      return -1;

    QWidget *stackItem = d->treeListToPageStack[selectedItems.first()];
    return d->pageToInt[stackItem];
  }
  else if (d->face == IconList) {
    QList<QListWidgetItem*> selectedItems = d->iconList->selectedItems();
    if (!selectedItems.count())
      return -1;

    QWidget *stackItem = d->iconListToPageStack[selectedItems.first()];
    return d->pageToInt[stackItem];
  }
  else if( d->face == Tabbed ) {
    QWidget *widget = d->tabControl->currentWidget();
    return ( !widget ? -1 : d->pageToInt[widget] );
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
  else if( d->face == TreeList || d->face == IconList )
  {
    return d->pageToInt[widget];
  }
  else if( d->face == Tabbed )
  {
    //
    // The user gets the real page widget with addVBoxPage(), addHBoxPage()
    // and addGridPage() but not with addPage() which returns a child of
    // the toplevel page. addPage() returns a QFrame so I check for that.
    //
    if( qobject_cast<QFrame*>(widget) )
    {
      return d->pageToInt[widget->parentWidget()];
    }
    else
    {
      return d->pageToInt[widget];
    }
  }
  else
  {
    return -1;
  }
}

void KJanusWidget::slotFontChanged()
{
  if( d->titleLabel )
  {
    d->titleLabel->setFont( KGlobalSettings::generalFont() );
    QFont titleFont( d->titleLabel->font() );
    titleFont.setBold( true );
    d->titleLabel->setFont( titleFont );
  }

  if( d->face == IconList )
  {
    QFont listFont( d->iconList->font() );
    listFont.setBold( true );
    d->iconList->setFont( listFont );
    d->iconList->invalidateHeight();
    d->iconList->invalidateWidth();
  }

  layout()->activate();
}

// makes the treelist behave like the list of kcontrol
void KJanusWidget::slotItemClicked(QTreeWidgetItem *it)
{
  if(it && (it->childCount()>0))
    it->treeWidget()->setItemExpanded(it, !it->treeWidget()->isItemExpanded(it));
}

void KJanusWidget::setFocus()
{
  if( !d->valid ) { return; }
  if( d->face == TreeList )
  {
    d->treeList->setFocus();
  }
  if( d->face == IconList )
  {
    d->iconList->setFocus();
  }
  else if( d->face == Tabbed )
  {
    d->tabControl->setFocus();
  }
  else if( d->face == Swallow )
  {
    d->swallowPage->setFocus();
  }
  else if( d->face == Plain )
  {
    d->plainPage->setFocus();
  }
}


/*QSize KJanusWidget::minimumSizeHint() const
{
  if( d->face == TreeList || d->face == IconList )
  {
    QSize s1( KDialog::spacingHint(), KDialog::spacingHint()*2 );
    QSize s2(0,0);
    QSize s3(0,0);
    QSize s4( d->pageStack->sizeHint() );

    if( d->face == TreeList )
    {
      s1.rwidth() += style()->pixelMetric( QStyle::PM_SplitterWidth );
      s2 = d->treeList->minimumSize();
    }
    else
    {
      d->iconList->updateMinimumHeight();
      d->iconList->updateWidth();
      s2 = d->iconList->minimumSize();
    }

    if( d->titleLabel->isVisible() )
    {
      s3 += d->titleLabel->sizeHint();
      s3.rheight() += d->titleSep->minimumSize().height();
    }

    //
    // Select the tallest item. It has only effect in IconList mode
    //
    int h1 = s1.rheight() + s3.rheight() + s4.height();
    int h2 = qMax( h1, s2.rheight() );

    return QSize( s1.width()+s2.width()+qMax(s3.width(),s4.width()), h2 );
  }
  else if( d->face == Tabbed )
  {
    return d->tabControl->sizeHint();
  }
  else if( d->face == Swallow )
  {
    return d->swallowPage->minimumSize();
  }
  else if( d->face == Plain )
  {
    return d->plainPage->sizeHint();
  }
  else
  {
    return QSize( 100, 100 ); // Should never happen though.
  }

}


QSize KJanusWidget::sizeHint() const
{
  return minimumSizeHint();
}*/


void KJanusWidget::setTreeListAutoResize( bool state )
{
  if( d->face == TreeList )
  {
    d->treeListResizeMode = !state ? KJanusWidgetPrivate::KeepSize : KJanusWidgetPrivate::Stretch;
    // the splitter's first widget is d->listFrame
    if( d->splitter && d->splitter->count() > 0 )
        d->splitter->setStretchFactor( 0, d->treeListResizeMode == KJanusWidgetPrivate::KeepSize ? 0 : 1 );

    layout()->activate();
  }
}


void KJanusWidget::setIconListAllVisible( bool state )
{
  if( d->face == IconList )
  {
    d->iconList->setShowAll( state );
  }
}

void KJanusWidget::setShowIconsInTreeList( bool state )
{
  d->showIconsInTreeList = state;
}

void KJanusWidget::setRootIsDecorated( bool state )
{
  if( d->face == TreeList ) {
    d->treeList->setRootIsDecorated(state);
    layout()->activate();
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
  if( d->face == TreeList )
  {
    if( persist ) {
      d->treeList->setItemsExpandable/*ByUser*/( false );
      d->treeList->setRootIsDecorated( false );
    }

    for (QTreeWidgetItemIterator it(d->treeList); *it; ++it)
      d->treeList->expandItem(*it);

    layout()->activate();

    kDebug() << k_funcinfo << d->treeList->sizeHint() << endl;
  }
}

void KJanusWidget::addWidgetBelowList( QWidget * widget )
{
  if( ( d->face == TreeList || d->face == IconList ) && d->listFrame )
  {
    widget->setParent( d->listFrame );
    d->listFrame->layout()->addWidget(widget);
  }
}

void KJanusWidget::addButtonBelowList( const QString & text, QObject * recv, const char * slot )
{
  if( ( d->face == TreeList || d->face == IconList ) && d->listFrame )
  {
    QPushButton * button = new QPushButton( text, d->listFrame);
    button->setObjectName( "KJanusWidget::buttonBelowList" );
    connect( button, SIGNAL( clicked() ), recv, slot );
    d->listFrame->layout()->addWidget(button);
  }
}

void KJanusWidget::addButtonBelowList( const KGuiItem & item, QObject * recv, const char * slot )
{
  if( ( d->face == TreeList || d->face == IconList ) && d->listFrame )
  {
    KPushButton * button = new KPushButton( item, d->listFrame );
    button->setObjectName( "KJanusWidget::buttonBelowList" );
    connect( button, SIGNAL( clicked() ), recv, slot );
    d->listFrame->layout()->addWidget(button);
  }
}

void KJanusWidget::showEvent( QShowEvent * )
{
  if( d->face == TreeList )
  {
    if( d->splitter && d->splitter->count() > 0 )
        d->splitter->setStretchFactor( 0, d->treeListResizeMode == KJanusWidgetPrivate::KeepSize ? 0 : 1 );
  }
}


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

    QBitmap mask( pix->width(), pix->height() );
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
  if (!d || !d->pageToInt.contains(page))
    return;

  int index = d->pageToInt[page];

  if ( d->face == TreeList )
  {
    QMutableMapIterator<QTreeWidgetItem*, QWidget *> it = d->treeListToPageStack;
    while (it.hasNext()) {
      it.next();
      if (it.value() == page)
      {
        delete it.key();
        it.remove();
        d->pageStack->removeWidget(page);
        d->intToTitle.remove(index);
        d->pageToInt.remove(page);
        d->intToPage.remove(index);
        break;
      }
    }
  }
  else if ( d->face == IconList )
  {
    QMutableMapIterator<QListWidgetItem*, QWidget *> it = d->iconListToPageStack;
    while (it.hasNext()) {
      it.next();
      if (it.value() == page)
      {
        delete it.key();
        it.remove();
        d->pageStack->removeWidget(page);
        d->intToTitle.remove(index);
        d->pageToInt.remove(page);
        d->intToPage.remove(index);
        break;
      }
    }
  }
  else // Tabbed
  {
    d->tabControl->removeTab(d->tabControl->indexOf(page));
    d->pageToInt.remove(page);
    d->intToPage.remove(index);
  }
}

QString KJanusWidget::pageTitle(int index) const
{
  if (!d || !d->intToTitle.contains(index))
    return QString();
  else
    return d->intToTitle[index];
}

QWidget *KJanusWidget::pageWidget(int index) const
{
  if (!d || !d->intToPage.contains(index))
    return 0;
  else
    return d->intToPage[index];
}

void KJanusWidget::slotCurrentChanged( int index )
{
  emit currentPageChanged(d->pageStack->widget(index));
}

#include "kjanuswidget.moc"
