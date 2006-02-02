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
#include <kinstance.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmimetype.h>
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


#include <unistd.h>
#include <kvbox.h>

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

template <> inline
KIcon::Group KConfigBase::readEntry( const char *pKey,
                                     const KIcon::Group& aDefault ) const
{
  return static_cast<KIcon::Group>(readEntry(pKey,int(aDefault)));
}

template <> inline
void KConfigBase::writeEntry( const char *pKey,
                              const KIcon::Group& aValue,
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
                          const QString& icon, KIcon::Group group )
    : Q3ListBoxPixmap( KIconLoader::unknown() /*, parent->listBox()*/ ),
      m_url( url ),
      m_pixmap( ),
      m_parent( parent ),
      m_appLocal( true ),d(new KUrlBarItemPrivate)
{
    init( icon, group, description, persistent );
}

KUrlBarItem::KUrlBarItem( KUrlBar *parent,
                          const KUrl& url, const QString& description,
                          const QString& icon, KIcon::Group group )
    : Q3ListBoxPixmap( KIconLoader::unknown() /*, parent->listBox()*/ ),
      m_url( url ),
      m_pixmap(  ),
      m_parent( parent ),
      m_appLocal( true ),d(new KUrlBarItemPrivate)
{
    init( icon, group, description, true /*persistent*/ );
}

void KUrlBarItem::init( const QString& icon, KIcon::Group group,
                        const QString& description, bool persistent )
{
    d->isPersistent = persistent;

    setCustomHighlighting( true );
    setIcon( icon, group );
    setDescription( description );
}

KUrlBarItem::~KUrlBarItem()
{
    delete d;
}

void KUrlBarItem::setURL( const KUrl& url )
{
    m_url = url;
    if ( m_description.isEmpty() )
        setText( url.fileName() );
}

void KUrlBarItem::setIcon( const QString& icon, KIcon::Group group )
{
    m_icon  = icon;
    m_group = group;

    if ( icon.isEmpty() )
        m_pixmap = KMimeType::pixmapForURL( m_url, 0, group, iconSize() );
    else
        m_pixmap = KGlobal::iconLoader()->loadIcon( icon, group, iconSize(),
                                                    KIcon::DefaultState );
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
    return m_toolTip.isEmpty() ? m_url.prettyURL() : m_toolTip;
}

int KUrlBarItem::iconSize() const
{
    return m_parent->iconSize();
}

void KUrlBarItem::paint( QPainter *p )
{
    Q3ListBox *box = listBox();
    int w = width( box );
    static const int margin = KDialog::spacingHint();

    // draw sunken selection
    if ( isCurrent() || isSelected() ) {
        int h = height( box );

        QBrush brush = box->colorGroup().brush( QColorGroup::Highlight );
        p->fillRect( 0, 0, w, h, brush );
        QPen pen = p->pen();
        QPen oldPen = pen;
        pen.setColor( box->colorGroup().mid() );
        p->setPen( pen );

        p->drawPoint( 0, 0 );
        p->drawPoint( 0, h - 1 );
        p->drawPoint( w - 1, 0 );
        p->drawPoint( w - 1, h - 1 );

        p->setPen( oldPen );
    }

    if ( m_parent->iconSize() < KIcon::SizeMedium ) {
        // small icon -> draw icon next to text

        // ### mostly cut & paste of QListBoxPixmap::paint() until Qt 3.1
        // (where it will properly use pixmap() instead of the internal pixmap)
        const QPixmap *pm = pixmap();
        int yPos = QMAX( 0, (height(box) - pm->height())/2 );

        p->drawPixmap( margin, yPos, *pm );
        if ( !text().isEmpty() ) {
            QFontMetrics fm = p->fontMetrics();
            if ( pm->height() < fm.height() )
                yPos = fm.ascent() + fm.leading()/2;
            else
                yPos = pm->height()/2 - fm.height()/2 + fm.ascent();

            yPos += margin;
            int stringWidth = box->width() - pm->width() - 2 - (margin * 2);
            QString visibleText = KStringHandler::rPixelSqueeze( text(), fm, stringWidth );
            int xPos = pm->width() + margin + 2;

            if ( isCurrent() || isSelected() ) {
                p->setPen( box->colorGroup().highlight().dark(115) );
                p->drawText( xPos + ( QApplication::isRightToLeft() ? -1 : 1),
                             yPos + 1, visibleText );
                p->setPen( box->colorGroup().highlightedText() );
            }

            p->drawText( xPos, yPos, visibleText );
        }
        // end cut & paste (modulo pixmap centering)
    }

    else {
        // big icons -> draw text below icon
        int y = margin;
        const QPixmap *pm = pixmap();

        if ( !pm->isNull() ) {
            int x = (w - pm->width()) / 2;
            x = QMAX( x, margin );
            p->drawPixmap( x, y, *pm );
        }

        if ( !text().isEmpty() ) {
            QFontMetrics fm = p->fontMetrics();
            y += pm->height() + fm.height() - fm.descent();

            int stringWidth = box->width() - (margin * 2);
            QString visibleText = KStringHandler::rPixelSqueeze( text(), fm, stringWidth );
            int x = (w - fm.width( visibleText )) / 2;
            x = QMAX( x, margin );

            if ( isCurrent() || isSelected() ) {
                p->setPen( box->colorGroup().highlight().dark(115) );
                p->drawText( x + ( QApplication::isRightToLeft() ? -1 : 1),
                             y + 1, visibleText );
                p->setPen( box->colorGroup().highlightedText() );
            }

            p->drawText( x, y, visibleText );
        }
    }
}

QSize KUrlBarItem::sizeHint() const
{
    int wmin = 0;
    int hmin = 0;
    const KUrlBarListBox *lb =static_cast<const KUrlBarListBox*>(listBox());

    if ( m_parent->iconSize() < KIcon::SizeMedium ) {
        wmin = Q3ListBoxPixmap::width( lb ) + KDialog::spacingHint() * 2;
        hmin = Q3ListBoxPixmap::height( lb ) + KDialog::spacingHint() * 2;
    }
    else {
        wmin = QMAX(lb->fontMetrics().width(text()), pixmap()->width()) + KDialog::spacingHint() * 2;
        hmin = lb->fontMetrics().lineSpacing() + pixmap()->height() + KDialog::spacingHint() * 2;
    }

    if ( lb->isVertical() )
        wmin = QMIN( wmin, lb->viewport()->sizeHint().width() );
    else
        hmin = QMIN( hmin, lb->viewport()->sizeHint().height() );

    return QSize( wmin, hmin );
}

int KUrlBarItem::width( const Q3ListBox *lb ) const
{
    if ( static_cast<const KUrlBarListBox *>( lb )->isVertical() )
        return QMAX( sizeHint().width(), lb->viewport()->width() );
    else
        return sizeHint().width();
}

int KUrlBarItem::height( const Q3ListBox *lb ) const
{
    if ( static_cast<const KUrlBarListBox *>( lb )->isVertical() )
        return sizeHint().height();
    else
        return QMAX( sizeHint().height(), lb->viewport()->height() );
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


KUrlBar::KUrlBar( bool useGlobalItems, QWidget *parent, const char *name, Qt::WFlags f )
    : QFrame( parent, name, f ),
      m_activeItem( 0L ),
      m_useGlobal( useGlobalItems ),
      m_isModified( false ),
      m_isImmutable( false ),
      m_listBox( 0L ),
      m_iconSize( KIcon::SizeMedium ),d(new KUrlBarPrivate())
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

KUrlBar::~KUrlBar()
{
    delete d;
}

KUrlBarItem * KUrlBar::insertItem(const KUrl& url, const QString& description,
                                  bool applicationLocal,
                                  const QString& icon, KIcon::Group group )
{
    KUrlBarItem *item = new KUrlBarItem(this, url, description, icon, group);
    item->setApplicationLocal( applicationLocal );
    m_listBox->insertItem( item );
    return item;
}

KUrlBarItem * KUrlBar::insertDynamicItem(const KUrl& url, const QString& description,
                                         const QString& icon, KIcon::Group group )
{
    KUrlBarItem *item = new KUrlBarItem(this, url, false, description, icon, group);
    m_listBox->insertItem( item );
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

void KUrlBar::setListBox( KUrlBarListBox *view )
{
    delete m_listBox;

    if ( !view ) {
        m_listBox = new KUrlBarListBox( this, "urlbar listbox" );
        setOrientation( Qt::Vertical );
    }
    else {
        m_listBox = view;
        if ( m_listBox->parentWidget() != this )
            m_listBox->reparent( this, QPoint(0,0) );
        m_listBox->resize( width(), height() );
    }

    m_listBox->setSelectionMode( KListBox::Single );
    paletteChange( palette() );
    m_listBox->setFocusPolicy( Qt::TabFocus );

    connect( m_listBox, SIGNAL( mouseButtonClicked( int, Q3ListBoxItem *, const QPoint & ) ),
             SLOT( slotSelected( int, Q3ListBoxItem * )));
    connect( m_listBox, SIGNAL( dropped( QDropEvent * )),
             this, SLOT( slotDropped( QDropEvent * )));
    connect( m_listBox, SIGNAL( contextMenuRequested( Q3ListBoxItem *,
                                                      const QPoint& )),
             SLOT( slotContextMenuRequested( Q3ListBoxItem *, const QPoint& )));
    connect( m_listBox, SIGNAL( returnPressed( Q3ListBoxItem * ) ),
             SLOT( slotSelected( Q3ListBoxItem * ) ));
}

void KUrlBar::setIconSize( int size )
{
    if ( size == m_iconSize )
        return;

    m_iconSize = size;

    // reload the icons with the new size
    KUrlBarItem *item = static_cast<KUrlBarItem*>( m_listBox->firstItem() );
    while ( item ) {
        item->setIcon( item->icon(), item->iconGroup() );
        item = static_cast<KUrlBarItem*>( item->next() );
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


void KUrlBar::paletteChange( const QPalette & )
{
    QPalette pal = palette();
    QColor selectedTextColor = pal.color( QPalette::Normal, QColorGroup::BrightText );
    QColor foreground = pal.color( QPalette::Normal, QColorGroup::Foreground );
    pal.setColor( QPalette::Normal,   QColorGroup::Base, Qt::gray );
    pal.setColor( QPalette::Normal,   QColorGroup::HighlightedText, selectedTextColor );
    pal.setColor( QPalette::Normal,   QColorGroup::Text, foreground );
    pal.setColor( QPalette::Inactive, QColorGroup::Base, Qt::gray );
    pal.setColor( QPalette::Inactive, QColorGroup::HighlightedText, selectedTextColor );
    pal.setColor( QPalette::Inactive, QColorGroup::Text, foreground );

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
            w = QMAX( w, sh.width() );
            h += sh.height();
        }
        else {
            w += sh.width();
            h = QMAX( h, sh.height() );
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

void KUrlBar::slotSelected( int button, Q3ListBoxItem *item )
{
    if ( button != Qt::LeftButton )
        return;

    slotSelected( item );
}

void KUrlBar::slotSelected( Q3ListBoxItem *item )
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

    QString u = url.url(-1);

    if ( m_activeItem && m_activeItem->url().url(-1) == u )
        return;

    bool hasURL = false;
    Q3ListBoxItem *item = m_listBox->firstItem();
    while ( item ) {
        if ( static_cast<KUrlBarItem*>( item )->url().url(-1) == u ) {
            m_activeItem = static_cast<KUrlBarItem*>( item );
            m_listBox->setCurrentItem( item );
            m_listBox->setSelected( item, true );
            hasURL = true;
            break;
        }
        item = item->next();
    }

    if ( !hasURL ) {
        m_activeItem = 0L;
        m_listBox->clearSelection();
    }
}

KUrlBarItem * KUrlBar::currentItem() const
{
    Q3ListBoxItem *item = m_listBox->item( m_listBox->currentItem() );
    if ( item )
        return static_cast<KUrlBarItem *>( item );
    return 0L;
}

KUrl KUrlBar::currentURL() const
{
    KUrlBarItem *item = currentItem();
    return item ? item->url() : KUrl();
}

void KUrlBar::readConfig( KConfig *appConfig, const QString& itemGroup )
{
    m_isImmutable = appConfig->groupIsImmutable( itemGroup );
    KConfigGroup appGroup( appConfig, itemGroup );
    d->defaultIconSize = m_iconSize;
    m_iconSize = appGroup.readEntry( "Speedbar IconSize", m_iconSize );

    if ( m_useGlobal ) { // read global items
        KConfigGroup globalGroup( KGlobal::config(), (QString)(itemGroup +" (Global)"));
        int num = globalGroup.readEntry( "Number of Entries",0 );
        for ( int i = 0; i < num; i++ ) {
            readItem( i, &globalGroup, false );
        }
    }

    // read application local items
    int num = appGroup.readEntry( "Number of Entries",0 );
    for ( int i = 0; i < num; i++ ) {
        readItem( i, &appGroup, true );
    }
}

void KUrlBar::readItem( int i, KConfigBase *config, bool applicationLocal )
{
    QString number = QString::number( i );
    KUrl url = KUrl::fromPathOrURL( config->readPathEntry( QString("URL_") + number ));
    if ( !url.isValid() || !KProtocolInfo::isKnownProtocol( url ))
        return; // nothing we could do.

    insertItem( url,
                config->readEntry( QString("Description_") + number, QString() ),
                applicationLocal,
                config->readEntry( QString("Icon_") + number, QString() ),
                config->readEntry( QString("IconGroup_") + number, KIcon::Group() ) );
}

void KUrlBar::writeConfig( KConfig *config, const QString& itemGroup )
{
    KConfigGroup cg( config, itemGroup );
    if(!cg.hasDefault("Speedbar IconSize") && m_iconSize == d->defaultIconSize )
        cg.revertToDefault("Speedbar IconSize");
    else
        cg.writeEntry( "Speedbar IconSize", m_iconSize );

    if ( !m_isModified )
        return;

    int i = 0;
    int numLocal = 0;
    KUrlBarItem *item = static_cast<KUrlBarItem*>( m_listBox->firstItem() );

    while ( item )
    {
        if ( item->isPersistent() ) // we only save persistent items
        {
            if ( item->applicationLocal() )
            {
                writeItem( item, numLocal, config, false );
                numLocal++;
            }

            i++;
        }
        item = static_cast<KUrlBarItem*>( item->next() );
    }
    cg.writeEntry("Number of Entries", numLocal);


    // write the global entries to kdeglobals, if any
    bool haveGlobalEntries = (i > numLocal);
    if ( m_useGlobal && haveGlobalEntries ) {
        config->setGroup( itemGroup + " (Global)" );

        int numGlobals = 0;
        item = static_cast<KUrlBarItem*>( m_listBox->firstItem() );

        while ( item )
        {
            if ( item->isPersistent() ) // we only save persistent items
            {
                if ( !item->applicationLocal() )
                {
                    writeItem( item, numGlobals, config, true );
                    numGlobals++;
                }
            }

            item = static_cast<KUrlBarItem*>( item->next() );
        }
        config->writeEntry("Number of Entries", numGlobals,
                           KConfigBase::Normal|KConfigBase::Global);
    }

    m_isModified = false;
}

void KUrlBar::writeItem( KUrlBarItem *item, int i, KConfig *config,
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
    config->writePathEntry( URL + number, item->url().prettyURL(), flags );
    config->writeEntry( Description + number, item->description(), flags );
    config->writeEntry( Icon + number, item->icon(), flags );
    config->writeEntry( IconGroup + number, item->iconGroup(), flags );
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

void KUrlBar::slotContextMenuRequested( Q3ListBoxItem *_item, const QPoint& pos )
{
    if (m_isImmutable)
        return;

    KUrlBarItem *item = dynamic_cast<KUrlBarItem*>( _item );

    KUrl lastURL = m_activeItem ? m_activeItem->url() : KUrl();

    bool smallIcons = m_iconSize < KIcon::SizeMedium;
    QMenu *popup = new QMenu();
    QAction* IconSize = popup->addAction( smallIcons ?
                       i18n("&Large Icons") : i18n("&Small Icons"));
    popup->addSeparator();

    QAction* EditItem = 0L;
    if (item != 0L && item->isPersistent())
    {
        EditItem = popup->addAction(SmallIconSet("edit"), i18n("&Edit Entry..."));
        popup->addSeparator();
    }

    QAction* AddItem = popup->addAction(SmallIconSet("filenew"), i18n("&Add Entry..."));

    QAction* RemoveItem = 0L;
    if (item != 0L && item->isPersistent())
        RemoveItem = popup->addAction( SmallIconSet("editdelete"), i18n("&Remove Entry"));

    QAction* result = popup->exec( pos );
    if (result == IconSize) {
        setIconSize( smallIcons ? KIcon::SizeMedium : KIcon::SizeSmallMedium );
        m_listBox->triggerUpdate( true );

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
        m_listBox->insertItem( item );
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
        item->setURL( url );
        item->setDescription( description );
        item->setIcon( icon );
        item->setApplicationLocal( appLocal );
        m_listBox->triggerUpdate( true );
        m_isModified = true;
        updateGeometry();
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


KUrlBarListBox::KUrlBarListBox( QWidget *parent, const char *name )
    : KListBox( parent, name )
{
    setAcceptDrops( true );
    viewport()->setAcceptDrops( true );
}

KUrlBarListBox::~KUrlBarListBox()
{
}

void KUrlBarListBox::paintEvent( QPaintEvent* )
{
    QPainter p(this);
    p.setPen( colorGroup().mid() );
    p.drawRect( 0, 0, width() - 1, height() - 1 );
}

void KUrlBarListBox::populateMimeData( QMimeData* mimeData )
{
    KUrl::List urls;
    KUrlBarItem *item = static_cast<KUrlBarItem*>( firstItem() );

    while ( item ) {
        if ( item->isSelected() )
            urls.append( item->url() );
        item = static_cast<KUrlBarItem*>( item->next() );
    }

    if ( !urls.isEmpty() ) {
        // ### use custom drag-object with description etc.?
        urls.populateMimeData( mimeData );
    }
}

void KUrlBarListBox::contentsDragEnterEvent( QDragEnterEvent *e )
{
    e->accept( KUrl::List::canDecode( e->mimeData() ) );
}

void KUrlBarListBox::contentsDropEvent( QDropEvent *e )
{
    emit dropped( e );
}

void KUrlBarListBox::contextMenuEvent( QContextMenuEvent *e )
{
    if (e)
    {
        emit contextMenuRequested( itemAt( e->globalPos() ), e->globalPos() );
        e->accept(); // Consume the event to avoid multiple contextMenuEvent calls...
    }
}

void KUrlBarListBox::setOrientation( Qt::Orientation orient )
{
    if ( orient == Qt::Vertical ) {
        setColumnMode( 1 );
        setRowMode( Variable );
    }
    else {
        setRowMode( 1 );
        setColumnMode( Variable );
    }

    m_orientation = orient;
}

bool KUrlBarListBox::event( QEvent* e )
{
    if ( e->type() == QEvent::ToolTip )
    {
        QHelpEvent* he = static_cast<QHelpEvent*>( e );
        Q3ListBoxItem *item = itemAt( he->pos() );
        if ( item ) {
            QString text = static_cast<KUrlBarItem*>( item )->toolTip();
            if ( !text.isEmpty() )
                QToolTip::showText( itemRect( item ).topLeft(), text, this );
        }

        return true;
    }

    return QFrame::event(e);
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
                                      QString icon, bool appLocal,
                                      int iconSize,
                                      QWidget *parent, const char *name )
    : KDialogBase( parent, name, true,
                   i18n("Edit Quick Access Entry"), Ok | Cancel, Ok, true )
{
    KVBox *box = new KVBox( this );
    QString text = i18n("<qt><b>Please provide a description, URL and icon for this Quick Access entry.</b></br></qt>");
    QLabel *label = new QLabel( text, box );
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
                         "appropriate URL.</qt>").arg(QDir::homePath());
    label = new QLabel( i18n("&URL:"), grid );
    m_urlEdit = new KUrlRequester( url.prettyURL(), grid );
    m_urlEdit->setMode( KFile::Directory );
    label->setBuddy( m_urlEdit );
    label->setWhatsThis(whatsThisText );
    m_urlEdit->setWhatsThis(whatsThisText );

    whatsThisText = i18n("<qt>This is the icon that will appear in the Quick Access panel.<p>"
                         "Click on the button to select a different icon.</qt>");
    label = new QLabel( i18n("Choose an &icon:"), grid );
    m_iconButton = new KIconButton( grid, "icon button" );
    m_iconButton->setIconSize( iconSize );
    if ( icon.isEmpty() )
        icon = KMimeType::iconNameForURL( url );
    m_iconButton->setIcon( icon );
    label->setBuddy( m_iconButton );
    label->setWhatsThis(whatsThisText );
    m_iconButton->setWhatsThis(whatsThisText );

    if ( allowGlobal ) {
        QString appName;
        if ( KGlobal::instance()->aboutData() )
            appName = KGlobal::instance()->aboutData()->programName();
        if ( appName.isEmpty() )
            appName = QLatin1String( KGlobal::instance()->instanceName() );
        m_appLocal = new QCheckBox( i18n("&Only show when using this application (%1)").arg( appName ), box );
        m_appLocal->setChecked( appLocal );
        m_appLocal->setWhatsThis(                         i18n("<qt>Select this setting if you want this "
                              "entry to show only when using the current application (%1).<p>"
                              "If this setting is not selected, the entry will be available in all "
                              "applications.</qt>")
                              .arg(appName));
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
    enableButtonOK( !text.isEmpty() );
}

KUrl KUrlBarItemDialog::url() const
{
    QString text = m_urlEdit->url();
    KUrl u;
    if ( text.at(0) == '/' )
        u.setPath( text );
    else
        u = text;

    return u;
}

QString KUrlBarItemDialog::description() const
{
    return m_edit->text();
}

QString KUrlBarItemDialog::icon() const
{
    return m_iconButton->icon();
}

bool KUrlBarItemDialog::applicationLocal() const
{
    if ( !m_appLocal )
        return true;

    return m_appLocal->isChecked();
}

void KUrlBarItem::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KUrlBar::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KUrlBarListBox::virtual_hook( int id, void* data )
{ KListBox::virtual_hook( id, data ); }


#include "kurlbar.moc"
