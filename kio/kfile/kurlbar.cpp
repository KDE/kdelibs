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
void KConfigBase::writeEntry( const char *pKey,
                              const KIcon::Group& aValue,
                              KConfigBase::WriteConfigFlags flags)
{
  writeEntry(pKey, int(aValue), flags);
}

class KURLBarItem::KURLBarItemPrivate
{
public:
    KURLBarItemPrivate()
    {
        isPersistent = true;
    }

    bool isPersistent;
};

KURLBarItem::KURLBarItem( KURLBar *parent,
                          const KURL& url, bool persistent, const QString& description,
                          const QString& icon, KIcon::Group group )
    : Q3ListBoxPixmap( KIconLoader::unknown() /*, parent->listBox()*/ ),
      m_url( url ),
      m_pixmap( ),
      m_parent( parent ),
      m_appLocal( true )
{
    init( icon, group, description, persistent );
}

KURLBarItem::KURLBarItem( KURLBar *parent,
                          const KURL& url, const QString& description,
                          const QString& icon, KIcon::Group group )
    : Q3ListBoxPixmap( KIconLoader::unknown() /*, parent->listBox()*/ ),
      m_url( url ),
      m_pixmap(  ),
      m_parent( parent ),
      m_appLocal( true )
{
    init( icon, group, description, true /*persistent*/ );
}

void KURLBarItem::init( const QString& icon, KIcon::Group group,
                        const QString& description, bool persistent )
{
    d = new KURLBarItemPrivate;
    d->isPersistent = persistent;

    setCustomHighlighting( true );
    setIcon( icon, group );
    setDescription( description );
}

KURLBarItem::~KURLBarItem()
{
    delete d;
}

void KURLBarItem::setURL( const KURL& url )
{
    m_url = url;
    if ( m_description.isEmpty() )
        setText( url.fileName() );
}

void KURLBarItem::setIcon( const QString& icon, KIcon::Group group )
{
    m_icon  = icon;
    m_group = group;

    if ( icon.isEmpty() )
        m_pixmap = KMimeType::pixmapForURL( m_url, 0, group, iconSize() );
    else
        m_pixmap = KGlobal::iconLoader()->loadIcon( icon, group, iconSize(),
                                                    KIcon::DefaultState );
}

void KURLBarItem::setDescription( const QString& desc )
{
    m_description = desc;
    setText( desc.isEmpty() ? m_url.fileName() : desc );
}

void KURLBarItem::setApplicationLocal( bool local )
{
    if ( !local && !isPersistent() )
    {
        kdWarning() << "KURLBar: dynamic (non-persistent) items can not be global." << endl;
        return;
    }

    m_appLocal = local;
}

void KURLBarItem::setToolTip( const QString& tip )
{
    m_toolTip = tip;
}

QString KURLBarItem::toolTip() const
{
    return m_toolTip.isEmpty() ? m_url.prettyURL() : m_toolTip;
}

int KURLBarItem::iconSize() const
{
    return m_parent->iconSize();
}

void KURLBarItem::paint( QPainter *p )
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

QSize KURLBarItem::sizeHint() const
{
    int wmin = 0;
    int hmin = 0;
    const KURLBarListBox *lb =static_cast<const KURLBarListBox*>(listBox());

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

int KURLBarItem::width( const Q3ListBox *lb ) const
{
    if ( static_cast<const KURLBarListBox *>( lb )->isVertical() )
        return QMAX( sizeHint().width(), lb->viewport()->width() );
    else
        return sizeHint().width();
}

int KURLBarItem::height( const Q3ListBox *lb ) const
{
    if ( static_cast<const KURLBarListBox *>( lb )->isVertical() )
        return sizeHint().height();
    else
        return QMAX( sizeHint().height(), lb->viewport()->height() );
}

bool KURLBarItem::isPersistent() const
{
    return d->isPersistent;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class KURLBar::KURLBarPrivate
{
public:
    KURLBarPrivate()
    {
        currentURL.setPath( QDir::homePath() );
        defaultIconSize = 0;
    }

    int defaultIconSize;
    KURL currentURL;
};


KURLBar::KURLBar( bool useGlobalItems, QWidget *parent, const char *name, Qt::WFlags f )
    : QFrame( parent, name, f ),
      m_activeItem( 0L ),
      m_useGlobal( useGlobalItems ),
      m_isModified( false ),
      m_isImmutable( false ),
      m_listBox( 0L ),
      m_iconSize( KIcon::SizeMedium )
{
    d = new KURLBarPrivate();

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

KURLBar::~KURLBar()
{
    delete d;
}

KURLBarItem * KURLBar::insertItem(const KURL& url, const QString& description,
                                  bool applicationLocal,
                                  const QString& icon, KIcon::Group group )
{
    KURLBarItem *item = new KURLBarItem(this, url, description, icon, group);
    item->setApplicationLocal( applicationLocal );
    m_listBox->insertItem( item );
    return item;
}

KURLBarItem * KURLBar::insertDynamicItem(const KURL& url, const QString& description,
                                         const QString& icon, KIcon::Group group )
{
    KURLBarItem *item = new KURLBarItem(this, url, false, description, icon, group);
    m_listBox->insertItem( item );
    return item;
}

void KURLBar::setOrientation( Qt::Orientation orient )
{
    m_listBox->setOrientation( orient );
    setSizePolicy( QSizePolicy( isVertical() ?
                                QSizePolicy::Maximum :
                                QSizePolicy::Preferred,
                                isVertical() ?
                                QSizePolicy::Preferred :
                                QSizePolicy::Maximum ));
}

Qt::Orientation KURLBar::orientation() const
{
    return m_listBox->orientation();
}

void KURLBar::setListBox( KURLBarListBox *view )
{
    delete m_listBox;

    if ( !view ) {
        m_listBox = new KURLBarListBox( this, "urlbar listbox" );
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

void KURLBar::setIconSize( int size )
{
    if ( size == m_iconSize )
        return;

    m_iconSize = size;

    // reload the icons with the new size
    KURLBarItem *item = static_cast<KURLBarItem*>( m_listBox->firstItem() );
    while ( item ) {
        item->setIcon( item->icon(), item->iconGroup() );
        item = static_cast<KURLBarItem*>( item->next() );
    }

    resize( sizeHint() );
    updateGeometry();
}

void KURLBar::clear()
{
    m_listBox->clear();
}

void KURLBar::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent( e );
    m_listBox->resize( width(), height() );
}


void KURLBar::paletteChange( const QPalette & )
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

QSize KURLBar::sizeHint() const
{
    return minimumSizeHint();

#if 0
    // this code causes vertical and or horizontal scrollbars appearing
    // depending on the text, font, moonphase and earth rotation. Just using
    // m_listBox->sizeHint() fixes this (although the widget can then be
    // resized to a smaller size so that scrollbars appear).
    int w = 0;
    int h = 0;
    KURLBarItem *item;
    bool vertical = isVertical();

    for ( item = static_cast<KURLBarItem*>( m_listBox->firstItem() );
          item;
          item = static_cast<KURLBarItem*>( item->next() ) ) {

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

QSize KURLBar::minimumSizeHint() const
{
    QSize s       = m_listBox->sizeHint();
    int   barSize = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    int w = s.width()  + barSize;
    int h = s.height() + barSize;
    return QSize( w, h );
}

void KURLBar::slotSelected( int button, Q3ListBoxItem *item )
{
    if ( button != Qt::LeftButton )
        return;

    slotSelected( item );
}

void KURLBar::slotSelected( Q3ListBoxItem *item )
{
    if ( item && item != m_activeItem )
        m_activeItem = static_cast<KURLBarItem*>( item );

    if ( m_activeItem ) {
        m_listBox->setCurrentItem( m_activeItem );
        emit activated( m_activeItem->url() );
    }
}

void KURLBar::setCurrentItem( const KURL& url )
{
    d->currentURL = url;

    QString u = url.url(-1);

    if ( m_activeItem && m_activeItem->url().url(-1) == u )
        return;

    bool hasURL = false;
    Q3ListBoxItem *item = m_listBox->firstItem();
    while ( item ) {
        if ( static_cast<KURLBarItem*>( item )->url().url(-1) == u ) {
            m_activeItem = static_cast<KURLBarItem*>( item );
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

KURLBarItem * KURLBar::currentItem() const
{
    Q3ListBoxItem *item = m_listBox->item( m_listBox->currentItem() );
    if ( item )
        return static_cast<KURLBarItem *>( item );
    return 0L;
}

KURL KURLBar::currentURL() const
{
    KURLBarItem *item = currentItem();
    return item ? item->url() : KURL();
}

void KURLBar::readConfig( KConfig *appConfig, const QString& itemGroup )
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

void KURLBar::readItem( int i, KConfigBase *config, bool applicationLocal )
{
    QString number = QString::number( i );
    KURL url = KURL::fromPathOrURL( config->readPathEntry( QString("URL_") + number ));
    if ( !url.isValid() || !KProtocolInfo::isKnownProtocol( url ))
        return; // nothing we could do.

    insertItem( url,
                config->readEntry( QString("Description_") + number, QString() ),
                applicationLocal,
                config->readEntry( QString("Icon_") + number, QString() ),
                static_cast<KIcon::Group>(
                    config->readEntry( QString("IconGroup_") + number,0 )) );
}

void KURLBar::writeConfig( KConfig *config, const QString& itemGroup )
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
    KURLBarItem *item = static_cast<KURLBarItem*>( m_listBox->firstItem() );

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
        item = static_cast<KURLBarItem*>( item->next() );
    }
    cg.writeEntry("Number of Entries", numLocal);


    // write the global entries to kdeglobals, if any
    bool haveGlobalEntries = (i > numLocal);
    if ( m_useGlobal && haveGlobalEntries ) {
        config->setGroup( itemGroup + " (Global)" );

        int numGlobals = 0;
        item = static_cast<KURLBarItem*>( m_listBox->firstItem() );

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

            item = static_cast<KURLBarItem*>( item->next() );
        }
        config->writeEntry("Number of Entries", numGlobals, KConfigBase::Global);
    }

    m_isModified = false;
}

void KURLBar::writeItem( KURLBarItem *item, int i, KConfig *config,
                         bool global )
{
    if ( !item->isPersistent() )
        return;

    QString Description = "Description_";
    QString URL = "URL_";
    QString Icon = "Icon_";
    QString IconGroup = "IconGroup_";

    QString number = QString::number( i );
    config->writePathEntry( URL + number, item->url().prettyURL(),  (global?KConfigBase::Global:KConfigBase::Normal) );
    config->writeEntry( Description + number, item->description(), (global?KConfigBase::Global:KConfigBase::Normal));
    config->writeEntry( Icon + number, item->icon(), (global?KConfigBase::Global:KConfigBase::Normal) );
    config->writeEntry( IconGroup + number, item->iconGroup(), (global?KConfigBase::Global:KConfigBase::Normal) );
}


void KURLBar::slotDropped( QDropEvent *e )
{
    KURL::List urls = KURL::List::fromMimeData( e->mimeData() );
    if ( !urls.isEmpty() ) {
        KURL url;
        QString description;
        QString icon;
        bool appLocal = false;

        KURL::List::const_iterator it = urls.begin();
        for ( ; it != urls.end(); ++it ) {
            (void) insertItem( *it, description, appLocal, icon );
            m_isModified = true;
            updateGeometry();
        }
    }
}

void KURLBar::slotContextMenuRequested( Q3ListBoxItem *_item, const QPoint& pos )
{
    if (m_isImmutable)
        return;

    KURLBarItem *item = dynamic_cast<KURLBarItem*>( _item );

    KURL lastURL = m_activeItem ? m_activeItem->url() : KURL();

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
        editItem( static_cast<KURLBarItem *>( item ) );

    } else if (result == RemoveItem) {
        delete item;
        m_isModified = true;
    }

    // reset current item
    m_activeItem = 0L;
    setCurrentItem( lastURL );
}

bool KURLBar::addNewItem()
{
    KURLBarItem *item = new KURLBarItem( this, d->currentURL,
                                         i18n("Enter a description") );
    if ( editItem( item ) ) {
        m_listBox->insertItem( item );
        return true;
    }

    delete item;
    return false;
}

bool KURLBar::editItem( KURLBarItem *item )
{
    if ( !item || !item->isPersistent() ) // should never happen tho
        return false;

    KURL url            = item->url();
    QString description = item->description();
    QString icon        = item->icon();
    bool appLocal       = item->applicationLocal();

    if ( KURLBarItemDialog::getInformation( m_useGlobal,
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


KURLBarListBox::KURLBarListBox( QWidget *parent, const char *name )
    : KListBox( parent, name )
{
    setAcceptDrops( true );
    viewport()->setAcceptDrops( true );
}

KURLBarListBox::~KURLBarListBox()
{
}

void KURLBarListBox::paintEvent( QPaintEvent* )
{
    QPainter p(this);
    p.setPen( colorGroup().mid() );
    p.drawRect( 0, 0, width() - 1, height() - 1 );
}

void KURLBarListBox::populateMimeData( QMimeData* mimeData )
{
    KURL::List urls;
    KURLBarItem *item = static_cast<KURLBarItem*>( firstItem() );

    while ( item ) {
        if ( item->isSelected() )
            urls.append( item->url() );
        item = static_cast<KURLBarItem*>( item->next() );
    }

    if ( !urls.isEmpty() ) {
        // ### use custom drag-object with description etc.?
        urls.populateMimeData( mimeData );
    }
}

void KURLBarListBox::contentsDragEnterEvent( QDragEnterEvent *e )
{
    e->accept( KURL::List::canDecode( e->mimeData() ) );
}

void KURLBarListBox::contentsDropEvent( QDropEvent *e )
{
    emit dropped( e );
}

void KURLBarListBox::contextMenuEvent( QContextMenuEvent *e )
{
    if (e)
    {
        emit contextMenuRequested( itemAt( e->globalPos() ), e->globalPos() );
        e->accept(); // Consume the event to avoid multiple contextMenuEvent calls...
    }
}

void KURLBarListBox::setOrientation( Qt::Orientation orient )
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

bool KURLBarListBox::event( QEvent* e )
{
    if ( e->type() == QEvent::ToolTip )
    {
        QHelpEvent* he = static_cast<QHelpEvent*>( e );
        Q3ListBoxItem *item = itemAt( he->pos() );
        if ( item ) {
            QString text = static_cast<KURLBarItem*>( item )->toolTip();
            if ( !text.isEmpty() )
                QToolTip::showText( itemRect( item ).topLeft(), text, this );
        }

        return true;
    }

    return QFrame::event(e);
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


bool KURLBarItemDialog::getInformation( bool allowGlobal, KURL& url,
                                        QString& description, QString& icon,
                                        bool& appLocal, int iconSize,
                                        QWidget *parent )
{
    KURLBarItemDialog *dialog = new KURLBarItemDialog( allowGlobal, url,
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

KURLBarItemDialog::KURLBarItemDialog( bool allowGlobal, const KURL& url,
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
    m_urlEdit = new KURLRequester( url.prettyURL(), grid );
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

KURLBarItemDialog::~KURLBarItemDialog()
{
}

void KURLBarItemDialog::urlChanged(const QString & text )
{
    enableButtonOK( !text.isEmpty() );
}

KURL KURLBarItemDialog::url() const
{
    QString text = m_urlEdit->url();
    KURL u;
    if ( text.at(0) == '/' )
        u.setPath( text );
    else
        u = text;

    return u;
}

QString KURLBarItemDialog::description() const
{
    return m_edit->text();
}

QString KURLBarItemDialog::icon() const
{
    return m_iconButton->icon();
}

bool KURLBarItemDialog::applicationLocal() const
{
    if ( !m_appLocal )
        return true;

    return m_appLocal->isChecked();
}

void KURLBarItem::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KURLBar::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KURLBarListBox::virtual_hook( int id, void* data )
{ KListBox::virtual_hook( id, data ); }


#include "kurlbar.moc"
