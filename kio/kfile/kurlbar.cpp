/* This file is part of the KDE libraries
    Copyright (C) 2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kurlbar.h"

#include <kaboutdata.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kicondialog.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kio/global.h>
#include <kprotocolinfo.h>
#include <kstringhandler.h>
#include <kurlrequester.h>

#include <qapplication.h>
#include <qcheckbox.h>
#include <qdrawutil.h>
#include <qfontmetrics.h>
#include <qlabel.h>
#include <qmimedata.h>
#include <q3grid.h>
#include <qpainter.h>
#include <qmenu.h>
#include <qstyle.h>
#include <qitemdelegate.h>

#include <unistd.h>
#include <kvbox.h>
#include <kconfiggroup.h>

class KUrlBarItemDelegate : public QItemDelegate
{
public:
    KUrlBarItemDelegate(QObject* parent) : QItemDelegate(parent) {}

    virtual void paint(QPainter* painter,const QStyleOptionViewItem& option,const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option , const QModelIndex& index) const;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

template <> inline
K3Icon::Group KConfigGroup::readEntry( const char *pKey,
                                     const K3Icon::Group& aDefault ) const
{
    return static_cast<K3Icon::Group>(readEntry(pKey,int(aDefault)));
}

template <> inline
void KConfigGroup::writeEntry( const char *pKey,
                              const K3Icon::Group& aValue,
                              KConfigBase::WriteConfigFlags flags)
{
    writeEntry(pKey, int(aValue), flags);
}

class KUrlBarItem::KUrlBarItemPrivate
{
public:
    KUrlBarItemPrivate()
    {
        isPersistent = true;
    }

    bool isPersistent;
};

KUrlBarItem::KUrlBarItem( KUrlBar *parent,
                          const KUrl& url, bool persistent, const QString& description,
                          const QString& icon, K3Icon::Group group )
    : QListWidgetItem(),
      m_url( url ),
      m_pixmap( ),
      m_parent( parent ),
      m_appLocal( true ),d(new KUrlBarItemPrivate)
{
    init( icon, group, description, persistent );
}

KUrlBarItem::KUrlBarItem( KUrlBar *parent,
                          const KUrl& url, const QString& description,
                          const QString& icon, K3Icon::Group group )
    : QListWidgetItem(),
      m_url( url ),
      m_pixmap(  ),
      m_parent( parent ),
      m_appLocal( true ),d(new KUrlBarItemPrivate)
{
    init( icon, group, description, true /*persistent*/ );
}

void KUrlBarItem::init( const QString& icon, K3Icon::Group group,
                        const QString& description, bool persistent )
{

    qDebug() << __FUNCTION__ << "created new KUrlBarItem with description" << description;

    d->isPersistent = persistent;

    setIcon( icon, group );
    setDescription( description );
}

KUrlBarItem::~KUrlBarItem()
{
    delete d;
}

void KUrlBarItem::setUrl( const KUrl& url )
{
    m_url = url;
    if ( m_description.isEmpty() )
        setText( url.fileName() );
}

void KUrlBarItem::setIcon( const QString& icon, K3Icon::Group group )
{
    m_icon  = icon;
    m_group = group;

    if ( icon.isEmpty() )
        QListWidgetItem::setIcon( KIO::pixmapForUrl( m_url, 0, group, iconSize() ) );
    else
        QListWidgetItem::setIcon( KIconLoader::global()->loadIcon( icon, group, iconSize(),
                                                    K3Icon::DefaultState ) ); 
}

void KUrlBarItem::setDescription( const QString& desc )
{
    m_description = desc;
    setText( desc.isEmpty() ? m_url.fileName() : desc );
}

void KUrlBarItem::setApplicationLocal( bool local )
{
    if ( !local && !isPersistent() )
    {
        kWarning() << "KUrlBar: dynamic (non-persistent) items can not be global." << endl;
        return;
    }

    m_appLocal = local;
}

void KUrlBarItem::setToolTip( const QString& tip )
{
    m_toolTip = tip;
}

QString KUrlBarItem::toolTip() const
{
    return m_toolTip.isEmpty() ? m_url.prettyUrl() : m_toolTip;
}

int KUrlBarItem::iconSize() const
{
    return m_parent->iconSize();
}

QSize KUrlBarItemDelegate::sizeHint(const QStyleOptionViewItem& option , const QModelIndex& index) const
{
    int width = 0;
    int height = 0;

    QString text = index.data(Qt::DisplayRole).toString();

    QFontMetrics metrics(option.font);

    width = metrics.width(text) + option.decorationSize.width() + 
                   KDialog::spacingHint() * 2;
    
    height = qMax( metrics.height() , option.decorationSize.height() ) +
                    KDialog::spacingHint() * 2;

    return QSize( width , height );
}

void KUrlBarItemDelegate::paint( QPainter *painter , const QStyleOptionViewItem& option , 
                                 const QModelIndex& index) const
{
    const QString text = index.data(Qt::DisplayRole).toString();
    const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();

    QRect decorationRect = option.rect;
    QRect textRect = option.rect;

    const int decorationMargin = (option.rect.height() -
                                  option.decorationSize.height()) / 2;

    decorationRect.setTop( decorationRect.top() + decorationMargin );
    decorationRect.setBottom( decorationRect.bottom() - decorationMargin );
    decorationRect.setLeft( KDialog::spacingHint() );
    decorationRect.setWidth( option.decorationSize.width() );
    
    textRect.setLeft(   decorationRect.right()  
                      + KDialog::spacingHint() );

    painter->drawPixmap( decorationRect , icon.pixmap(decorationRect.size()) );
    painter->drawText( textRect , Qt::AlignVCenter , text );
}

int KUrlBarItem::width( const QListWidget *lb ) const
{
    if ( static_cast<const KUrlBarListWidget *>( lb )->isVertical() )
        return qMax( sizeHint().width(), lb->viewport()->width() );
    else
        return sizeHint().width();
}

int KUrlBarItem::height( const QListWidget *lb ) const
{
    if ( static_cast<const KUrlBarListWidget *>( lb )->isVertical() )
        return sizeHint().height();
    else
        return qMax( sizeHint().height(), lb->viewport()->height() );
}

bool KUrlBarItem::isPersistent() const
{
    return d->isPersistent;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class KUrlBar::KUrlBarPrivate
{
public:
    KUrlBarPrivate()
    {
        currentURL.setPath( QDir::homePath() );
        defaultIconSize = 0;
    }

    int defaultIconSize;
    KUrl currentURL;
};


KUrlBar::KUrlBar( bool useGlobalItems, QWidget *parent, Qt::WFlags f )
    : QFrame( parent, f ),
      m_activeItem( 0L ),
      m_useGlobal( useGlobalItems ),
      m_isModified( false ),
      m_isImmutable( false ),
      m_listBox( 0L ),
      m_iconSize( K3Icon::SizeMedium ),d(new KUrlBarPrivate())
{
    setListBox( 0L );
    setSizePolicy( QSizePolicy( isVertical() ?
                                QSizePolicy::Maximum :
                                QSizePolicy::Preferred,
                                isVertical() ?
                                QSizePolicy::Preferred :
                                QSizePolicy::Maximum ));
    this->setWhatsThis(i18n("<qt>The <b>Quick Access</b> panel provides easy access to commonly used file locations.<p>"
                               "Clicking on one of the shortcut entries will take you to that location.<p>"
                               "By right clicking on an entry you can add, edit and remove shortcuts.</qt>"));
}

KUrlBar::KUrlBar( bool useGlobalItems, QWidget *parent, const char *name, Qt::WFlags f )
    : QFrame( parent, f ),
      m_activeItem( 0L ),
      m_useGlobal( useGlobalItems ),
      m_isModified( false ),
      m_isImmutable( false ),
      m_listBox( 0L ),
      m_iconSize( K3Icon::SizeMedium ),d(new KUrlBarPrivate())
{
    setObjectName( name );
    setListBox( 0L );
    setSizePolicy( QSizePolicy( isVertical() ?
                                QSizePolicy::Maximum :
                                QSizePolicy::Preferred,
                                isVertical() ?
                                QSizePolicy::Preferred :
                                QSizePolicy::Maximum ));
    this->setWhatsThis(i18n("<qt>The <b>Quick Access</b> panel provides easy access to commonly used file locations.<p>"
                               "Clicking on one of the shortcut entries will take you to that location.<p>"
                               "By right clicking on an entry you can add, edit and remove shortcuts.</qt>"));
}

KUrlBar::~KUrlBar()
{
    delete d;
}

KUrlBarItem * KUrlBar::insertItem(const KUrl& url, const QString& description,
                                  bool applicationLocal,
                                  const QString& icon, K3Icon::Group group )
{
    KUrlBarItem *item = new KUrlBarItem(this, url, description, icon, group);
    item->setApplicationLocal( applicationLocal );
    m_listBox->addItem( item );
    return item;
}

KUrlBarItem * KUrlBar::insertDynamicItem(const KUrl& url, const QString& description,
                                         const QString& icon, K3Icon::Group group )
{
    KUrlBarItem *item = new KUrlBarItem(this, url, false, description, icon, group);
    m_listBox->addItem( item );
    return item;
}

void KUrlBar::setOrientation( Qt::Orientation orient )
{
    m_listBox->setOrientation( orient );
    setSizePolicy( QSizePolicy( isVertical() ?
                                QSizePolicy::Maximum :
                                QSizePolicy::Preferred,
                                isVertical() ?
                                QSizePolicy::Preferred :
                                QSizePolicy::Maximum ));
}

Qt::Orientation KUrlBar::orientation() const
{
    return m_listBox->orientation();
}

void KUrlBar::setListBox( KUrlBarListWidget *view )
{
    delete m_listBox;

    if ( !view ) {
        m_listBox = new KUrlBarListWidget( this );
        m_listBox->setObjectName("urlbar listbox" );
        setOrientation( Qt::Vertical );
    }
    else {
        m_listBox = view;
        if ( m_listBox->parentWidget() != this )
            m_listBox->setParent( this );
        m_listBox->resize( width(), height() );
    }

    m_listBox->setSelectionMode( QAbstractItemView::SingleSelection );
    paletteChange( palette() );
    m_listBox->setFocusPolicy( Qt::TabFocus );

    m_listBox->setFrameStyle( QFrame::Plain | QFrame::Box );

    connect( m_listBox, SIGNAL( itemClicked(QListWidgetItem *) ),
             SLOT( slotSelected(QListWidgetItem *)));


#ifdef __GNUC__
#warning "KDE4 - Port the drag/drop handling."
#endif

    // These signals refer to the Q3ListBox class and are not
    // available in QListWidget.  They need to be ported

    m_listBox->setContextMenuPolicy(Qt::CustomContextMenu);
    
    connect( m_listBox, SIGNAL( dropped( QDropEvent * )),
             this, SLOT( slotDropped( QDropEvent * )));
    connect( m_listBox, SIGNAL( customContextMenuRequested(const QPoint &)),
             SLOT( slotContextMenuRequested( const QPoint& )));
    
    connect( m_listBox, SIGNAL( returnPressed( QListWidgetItem * ) ),
             SLOT( slotSelected( QListWidgetItem * ) ));

    // --  
}

void KUrlBar::setIconSize( int size )
{
    if ( size == m_iconSize )
        return;

    m_iconSize = size;

    // reload the icons with the new size
    for ( int rowIndex = 0 ; rowIndex < m_listBox->count() ; rowIndex++ ) {
        KUrlBarItem *item = static_cast<KUrlBarItem*>( m_listBox->item(rowIndex) );
        item->setIcon( item->icon(), item->iconGroup() );
    }

    resize( sizeHint() );
    updateGeometry();
}

void KUrlBar::clear()
{
    m_listBox->clear();
}

void KUrlBar::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent( e );
    m_listBox->resize( width(), height() );
}


void KUrlBar::paletteChange( const QPalette & pal )
{
    /*QPalette pal = palette();
    QColor selectedTextColor = pal.color( QPalette::Normal, QPalette::BrightText );
    QColor foreground = pal.color( QPalette::Normal, QPalette::Foreground );
    pal.setColor( QPalette::Normal,   QPalette::Base, Qt::gray );
    pal.setColor( QPalette::Normal,   QPalette::HighlightedText, selectedTextColor );
    pal.setColor( QPalette::Normal,   QPalette::Text, foreground );
    pal.setColor( QPalette::Inactive, QPalette::Base, Qt::gray );
    pal.setColor( QPalette::Inactive, QPalette::HighlightedText, selectedTextColor );
    pal.setColor( QPalette::Inactive, QPalette::Text, foreground );
*/
    setPalette( pal );
}

QSize KUrlBar::sizeHint() const
{
    return minimumSizeHint();

#if 0
    // this code causes vertical and or horizontal scrollbars appearing
    // depending on the text, font, moonphase and earth rotation. Just using
    // m_listBox->sizeHint() fixes this (although the widget can then be
    // resized to a smaller size so that scrollbars appear).
    int w = 0;
    int h = 0;
    KUrlBarItem *item;
    bool vertical = isVertical();

    for ( item = static_cast<KUrlBarItem*>( m_listBox->firstItem() );
          item;
          item = static_cast<KUrlBarItem*>( item->next() ) ) {

        QSize sh = item->sizeHint();

        if ( vertical ) {
            w = qMax( w, sh.width() );
            h += sh.height();
        }
        else {
            w += sh.width();
            h = qMax( h, sh.height() );
        }
    }

//     if ( vertical && m_listBox->verticalScrollBar()->isVisible() )
//         w += m_listBox->verticalScrollBar()->width();
//     else if ( !vertical && m_listBox->horizontalScrollBar()->isVisible() )
//         h += m_listBox->horizontalScrollBar()->height();

    if ( w == 0 && h == 0 )
        return QSize( 100, 200 );
    else
        return QSize( 6 + w, h );
#endif
}

QSize KUrlBar::minimumSizeHint() const
{
    QSize s       = m_listBox->sizeHint();
    int   barSize = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    int w = s.width()  + barSize;
    int h = s.height() + barSize;
    return QSize( w, h );
}

void KUrlBar::slotSelected( QListWidgetItem *item )
{
    if ( item && item != m_activeItem )
        m_activeItem = static_cast<KUrlBarItem*>( item );

    if ( m_activeItem ) {
        m_listBox->setCurrentItem( m_activeItem );
        emit activated( m_activeItem->url() );
    }
}

void KUrlBar::setCurrentItem( const KUrl& url )
{
    d->currentURL = url;

    QString u = url.url(KUrl::RemoveTrailingSlash);

    if ( m_activeItem && m_activeItem->url().url(KUrl::RemoveTrailingSlash) == u )
        return;

    bool hasURL = false;
    
    for (int rowIndex = 0 ; rowIndex < m_listBox->count() ; rowIndex++ )
    {
        QListWidgetItem *item = m_listBox->item(rowIndex);
    
      if ( static_cast<KUrlBarItem*>( item )->url().url(KUrl::RemoveTrailingSlash) == u ) {
            m_activeItem = static_cast<KUrlBarItem*>( item );
            m_listBox->setCurrentItem( item );
            item->setSelected(true);
            hasURL = true;
            break;
        }
    }

    if ( !hasURL ) {
        m_activeItem = 0L;
        m_listBox->clearSelection();
    }
}

KUrlBarItem * KUrlBar::currentItem() const
{
    QListWidgetItem *item = m_listBox->item( m_listBox->currentRow() );
    if ( item )
        return static_cast<KUrlBarItem *>( item );
    return 0L;
}

KUrl KUrlBar::currentUrl() const
{
    KUrlBarItem *item = currentItem();
    return item ? item->url() : KUrl();
}

void KUrlBar::readConfig( const KConfigGroup& appGroup )
{
    m_isImmutable = appGroup.isImmutable();
    d->defaultIconSize = m_iconSize;
    m_iconSize = appGroup.readEntry( "Speedbar IconSize", m_iconSize );

    if ( m_useGlobal ) { // read global items
        KConfigGroup globalGroup( KGlobal::config(), appGroup.group() +" (Global)");
        int num = globalGroup.readEntry( "Number of Entries",0 );
        for ( int i = 0; i < num; i++ ) {
            readItem( i, globalGroup, false );
        }
    }

    // read application local items
    int num = appGroup.readEntry( "Number of Entries",0 );
    for ( int i = 0; i < num; i++ ) {
        readItem( i, appGroup, true );
    }
}

void KUrlBar::readItem( int i, const KConfigGroup &config, bool applicationLocal )
{
    QString number = QString::number( i );
    KUrl url( config.readPathEntry( QString(QLatin1String("URL_") + number).toUtf8().constData() ));
    if ( !url.isValid() || !KProtocolInfo::isKnownProtocol( url ))
        return; // nothing we could do.

    insertItem( url,
                config.readEntry( QString("Description_") + number, QString() ),
                applicationLocal,
                config.readEntry( QString("Icon_") + number, QString() ),
                config.readEntry( QString("IconGroup_") + number, K3Icon::Group() ) );
}

void KUrlBar::writeConfig( KConfigGroup &itemGroup )
{
    if(!itemGroup.hasDefault("Speedbar IconSize") && m_iconSize == d->defaultIconSize )
        itemGroup.revertToDefault("Speedbar IconSize");
    else
        itemGroup.writeEntry( "Speedbar IconSize", m_iconSize );

    if ( !m_isModified )
        return;

    int i = 0;
    int numLocal = 0;
    
    for ( int rowIndex = 0 ; rowIndex < m_listBox->count() ; rowIndex++ )
    {
        KUrlBarItem *item = static_cast<KUrlBarItem*>( m_listBox->item(rowIndex) );

        if ( item->isPersistent() ) // we only save persistent items
        {
            if ( item->applicationLocal() )
            {
                writeItem( item, numLocal, itemGroup, false );
                numLocal++;
            }

            i++;
        }
    }
    itemGroup.writeEntry("Number of Entries", numLocal);


    // write the global entries to kdeglobals, if any
    bool haveGlobalEntries = (i > numLocal);
    if ( m_useGlobal && haveGlobalEntries ) {
        KConfigGroup gl = itemGroup;
        gl.changeGroup( itemGroup.group() + " (Global)" );

        int numGlobals = 0;

        for ( int rowIndex = 0 ; rowIndex < m_listBox->count() ; rowIndex++ )
        {
            KUrlBarItem* urlBarItem = static_cast<KUrlBarItem*>( m_listBox->item(rowIndex) );

            if ( urlBarItem->isPersistent() ) // we only save persistent items
            {
                if ( !urlBarItem->applicationLocal() )
                {
                    writeItem( urlBarItem, numGlobals, gl, true );
                    numGlobals++;
                }
            }

        }
        gl.writeEntry("Number of Entries", numGlobals,
                           KConfigBase::Normal|KConfigBase::Global);
    }

    m_isModified = false;
}

void KUrlBar::writeItem( KUrlBarItem *item, int i, KConfigGroup &config,
                         bool global )
{
    if ( !item->isPersistent() )
        return;

    QString Description = "Description_";
    QString URL = "URL_";
    QString Icon = "Icon_";
    QString IconGroup = "IconGroup_";

    QString number = QString::number( i );
    KConfigBase::WriteConfigFlags flags = KConfigBase::Normal;
    if ( global )
        flags |= KConfigBase::Global;
    config.writePathEntry( QString(URL + number).toUtf8().constData(), item->url().prettyUrl(), flags );
    config.writeEntry( QString(Description + number).toUtf8(), item->description(), flags );
    config.writeEntry( QString(Icon + number).toUtf8(), item->icon(), flags );
    config.writeEntry( QString(IconGroup + number).toUtf8().data(), item->iconGroup(), flags );
}


void KUrlBar::slotDropped( QDropEvent *e )
{
    KUrl::List urls = KUrl::List::fromMimeData( e->mimeData() );
    if ( !urls.isEmpty() ) {
        KUrl url;
        QString description;
        QString icon;
        bool appLocal = false;

        KUrl::List::const_iterator it = urls.begin();
        for ( ; it != urls.end(); ++it ) {
            (void) insertItem( *it, description, appLocal, icon );
            m_isModified = true;
            updateGeometry();
        }
    }
}

void KUrlBar::slotContextMenuRequested( const QPoint& pos )
{
    if (m_isImmutable)
        return;

    QListWidgetItem *_item = m_listBox->itemAt(pos);

    KUrlBarItem *item = dynamic_cast<KUrlBarItem*>( _item );

    KUrl lastURL = m_activeItem ? m_activeItem->url() : KUrl();

    bool smallIcons = m_iconSize < K3Icon::SizeMedium;
    QMenu *popup = new QMenu();
    QAction* IconSize = popup->addAction( smallIcons ?
                       i18n("&Large Icons") : i18n("&Small Icons"));
    popup->addSeparator();

    QAction* EditItem = 0L;
    if (item != 0L && item->isPersistent())
    {
        EditItem = popup->addAction(KIcon("edit"), i18n("&Edit Entry..."));
        popup->addSeparator();
    }

    QAction* AddItem = popup->addAction(KIcon("document-new"), i18n("&Add Entry..."));

    QAction* RemoveItem = 0L;
    if (item != 0L && item->isPersistent())
        RemoveItem = popup->addAction( KIcon("edit-delete"), i18n("&Remove Entry"));

    QAction* result = popup->exec( m_listBox->mapToGlobal(pos) );
    if (result == IconSize) {
        setIconSize( smallIcons ? K3Icon::SizeMedium : K3Icon::SizeSmallMedium );

    } else if (result == AddItem) {
        addNewItem();

    } else if (result == EditItem) {
        editItem( static_cast<KUrlBarItem *>( item ) );

    } else if (result == RemoveItem) {
        delete item;
        m_isModified = true;
    }

    // reset current item
    m_activeItem = 0L;
    setCurrentItem( lastURL );
}

bool KUrlBar::addNewItem()
{
    KUrlBarItem *item = new KUrlBarItem( this, d->currentURL,
                                         i18n("Enter a description") );
    if ( editItem( item ) ) {
        m_listBox->addItem( item );
        return true;
    }

    delete item;
    return false;
}

bool KUrlBar::editItem( KUrlBarItem *item )
{
    if ( !item || !item->isPersistent() ) // should never happen tho
        return false;

    KUrl url            = item->url();
    QString description = item->description();
    QString icon        = item->icon();
    bool appLocal       = item->applicationLocal();

    if ( KUrlBarItemDialog::getInformation( m_useGlobal,
                                            url, description,
                                            icon, appLocal,
                                            m_iconSize, this ))
    {
        item->setUrl( url );
        item->setDescription( description );
        item->setIcon( icon );
        item->setApplicationLocal( appLocal );
        m_isModified = true;
        updateGeometry();
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


KUrlBarListWidget::KUrlBarListWidget( QWidget *parent )
    : KListWidget( parent )
{
    setItemDelegate( new KUrlBarItemDelegate(this) );
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops( true );
    viewport()->setAcceptDrops( true );
}

KUrlBarListWidget::~KUrlBarListWidget()
{
}

/*void KUrlBarListWidget::paintEvent( QPaintEvent* )
{
    QPainter p(this);
    p.setPen( palette().color( QPalette::Mid ) );
    p.drawRect( 0, 0, width() - 1, height() - 1 );
}*/

void KUrlBarListWidget::populateMimeData( QMimeData* mimeData )
{
    KUrl::List urls;
    
    for (int rowIndex = 0 ; rowIndex < count() ; rowIndex++)
    {
        KUrlBarItem *urlBarItem = static_cast<KUrlBarItem*>( item(rowIndex) );

        if ( urlBarItem->isSelected() )
            urls.append( urlBarItem->url() );
    }

    if ( !urls.isEmpty() ) {
        // ### use custom drag-object with description etc.?
        urls.populateMimeData( mimeData );
    }
}

void KUrlBarListWidget::contentsDragEnterEvent( QDragEnterEvent *e )
{
    e->setAccepted( KUrl::List::canDecode( e->mimeData() ) );
}

void KUrlBarListWidget::contentsDropEvent( QDropEvent *e )
{
    emit dropped( e );
}

/*void KUrlBarListWidget::contextMenuEvent( QContextMenuEvent *e )
{
    if (e)
    {
        emit contextMenuRequested( item );
        e->accept(); // Consume the event to avoid multiple contextMenuEvent calls...
    }
}*/

void KUrlBarListWidget::setOrientation( Qt::Orientation orient )
{
    if ( orient == Qt::Vertical ) {
        //TODO KDE4 - Port me: setColumnMode( 1 );
    }
    else {
        //TODO KDE4 - Port me: setRowMode( 1 );
    }

    m_orientation = orient;
}

bool KUrlBarListWidget::event( QEvent* e )
{
    if ( e->type() == QEvent::ToolTip )
    {
        QHelpEvent* he = static_cast<QHelpEvent*>( e );
        QListWidgetItem *item = itemAt( he->pos() );
        if ( item ) {
            QString text = static_cast<KUrlBarItem*>( item )->toolTip();
            if ( !text.isEmpty() )
                QToolTip::showText( he->globalPos(), text, this );
        }

        return true;
    }

    return KListWidget::event(e);
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


bool KUrlBarItemDialog::getInformation( bool allowGlobal, KUrl& url,
                                        QString& description, QString& icon,
                                        bool& appLocal, int iconSize,
                                        QWidget *parent )
{
    KUrlBarItemDialog *dialog = new KUrlBarItemDialog( allowGlobal, url,
                                                       description, icon,
                                                       appLocal,
                                                       iconSize, parent );
    if ( dialog->exec() == QDialog::Accepted ) {
        // set the return parameters
        url         = dialog->url();
        description = dialog->description();
        icon        = dialog->icon();
        appLocal    = dialog->applicationLocal();

        delete dialog;
        return true;
    }

    delete dialog;
    return false;
}

KUrlBarItemDialog::KUrlBarItemDialog( bool allowGlobal, const KUrl& url,
                                      const QString& description,
                                      const QString &icon, bool appLocal,
                                      int iconSize,
                                      QWidget *parent)
    : KDialog( parent )
{
    setCaption( i18n("Edit Quick Access Entry") );
    setButtons( Ok | Cancel );
    setModal(true);
    setDefaultButton(Ok);
    showButtonSeparator(true);
    KVBox *box = new KVBox( this );
    QString text = i18n("<qt><b>Please provide a description, URL and icon for this Quick Access entry.</b></br></qt>");
    QLabel *label = new QLabel( text, box );
    label->setWordWrap(true);
    box->setSpacing( spacingHint() );

    Q3Grid *grid = new Q3Grid( 2, box );
    grid->setSpacing( spacingHint() );

    QString whatsThisText = i18n("<qt>This is the text that will appear in the Quick Access panel.<p>"
                                 "The description should consist of one or two words "
                                 "that will help you remember what this entry refers to.</qt>");
    label = new QLabel( i18n("&Description:"), grid );
    m_edit = new KLineEdit( grid);
    m_edit->setText( description.isEmpty() ? url.fileName() : description );
    label->setBuddy( m_edit );
    label->setWhatsThis(whatsThisText );
    m_edit->setWhatsThis(whatsThisText );

    whatsThisText = i18n("<qt>This is the location associated with the entry. Any valid URL may be used. For example:<p>"
                         "%1<br>http://www.kde.org<br>ftp://ftp.kde.org/pub/kde/stable<p>"
                         "By clicking on the button next to the text edit box you can browse to an "
                         "appropriate URL.</qt>", QDir::homePath());
    label = new QLabel( i18n("&URL:"), grid );
    m_urlEdit = new KUrlRequester( url.prettyUrl(), grid );
    m_urlEdit->setMode( KFile::Directory );
    label->setBuddy( m_urlEdit );
    label->setWhatsThis(whatsThisText );
    m_urlEdit->setWhatsThis(whatsThisText );

    whatsThisText = i18n("<qt>This is the icon that will appear in the Quick Access panel.<p>"
                         "Click on the button to select a different icon.</qt>");
    label = new QLabel( i18n("Choose an &icon:"), grid );
    m_iconButton = new KIconButton( grid );
    m_iconButton->setObjectName( QLatin1String( "icon button" ) );
    m_iconButton->setIconSize( iconSize );
    if ( icon.isEmpty() )
        m_iconButton->setIcon( KMimeType::iconNameForUrl( url ) );
    else
        m_iconButton->setIcon( icon );
    label->setBuddy( m_iconButton );
    label->setWhatsThis(whatsThisText );
    m_iconButton->setWhatsThis(whatsThisText );

    if ( allowGlobal ) {
        QString appName;
        if ( KGlobal::mainComponent().aboutData() )
            appName = KGlobal::mainComponent().aboutData()->programName();
        if ( appName.isEmpty() )
            appName = QLatin1String( KGlobal::mainComponent().componentName() );
        m_appLocal = new QCheckBox( i18n("&Only show when using this application (%1)",  appName ), box );
        m_appLocal->setChecked( appLocal );
        m_appLocal->setWhatsThis(i18n("<qt>Select this setting if you want this "
                              "entry to show only when using the current application (%1).<p>"
                              "If this setting is not selected, the entry will be available in all "
                              "applications.</qt>",
                               appName));
    }
    else
        m_appLocal = 0L;
    connect(m_urlEdit->lineEdit(),SIGNAL(textChanged ( const QString & )),this,SLOT(urlChanged(const QString & )));
    m_edit->setFocus();
    setMainWidget( box );
}

KUrlBarItemDialog::~KUrlBarItemDialog()
{
}

void KUrlBarItemDialog::urlChanged(const QString & text )
{
    enableButtonOk( !text.isEmpty() );
}

KUrl KUrlBarItemDialog::url() const
{
    return m_urlEdit->url();
}

QString KUrlBarItemDialog::description() const
{
    return m_edit->text();
}

const QString &KUrlBarItemDialog::icon() const
{
    return m_iconButton->icon();
}

bool KUrlBarItemDialog::applicationLocal() const
{
    if ( !m_appLocal )
        return true;

    return m_appLocal->isChecked();
}


#include "kurlbar.moc"
