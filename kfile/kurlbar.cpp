/* This file is part of the KDE libraries
    Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qcheckbox.h>
#include <qdrawutil.h>
#include <qfontmetrics.h>
#include <qlabel.h>
#include <qgrid.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qwhatsthis.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kicondialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kprotocolinfo.h>
#include <kurldrag.h>
#include <kurlrequester.h>

#include "kurlbar.h"

/**
 * Handles tooltips in the KURLBar
 * @internal
 */
class KURLBarToolTip : public QToolTip
{
public:
    KURLBarToolTip( QListBox *view ) : QToolTip( view ), m_view( view ) {}

protected:
    virtual void maybeTip( const QPoint& point ) {
        QListBoxItem *item = m_view->itemAt( point );
        if ( item ) {
            QString text = static_cast<KURLBarItem*>( item )->toolTip();
            if ( !text.isEmpty() )
                tip( m_view->itemRect( item ), text );
        }
    }

private:
    QListBox *m_view;
};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


KURLBarItem::KURLBarItem( KURLBar *parent,
                          const KURL& url, const QString& description,
                          const QString& icon, int group )
    : QListBoxPixmap( KIconLoader::unknown() /*, parent->listBox()*/ ),
      m_url( url ),
      m_pixmap( 0L ),
      m_parent( parent ),
      m_appLocal( true )
{
    setCustomHighlighting( true );
    setDescription( description );
    setIcon( icon, group );
}

KURLBarItem::~KURLBarItem()
{
}

void KURLBarItem::setURL( const KURL& url )
{
    m_url = url;
    if ( m_description.isEmpty() )
        setText( url.fileName() );
}

void KURLBarItem::setIcon( const QString& icon, int group )
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
    QListBox *box = listBox();
    int w = width( box );

    if ( m_parent->iconSize() < KIcon::SizeMedium ) {
        // small icon -> draw icon next to text

        // ### mostly cut & paste of QListBoxPixmap::paint() until Qt 3.1
        // (where it will properly use pixmap() instead of the internal pixmap)
        const QPixmap *pm = pixmap();
        int yPos = QMAX( 0, (height(box) - pm->height())/2 );

        p->drawPixmap( 3, yPos, *pm );
        if ( !text().isEmpty() ) {
            QFontMetrics fm = p->fontMetrics();
            if ( pm->height() < fm.height() )
                yPos = fm.ascent() + fm.leading()/2;
            else
                yPos = pm->height()/2 - fm.height()/2 + fm.ascent();
            p->drawText( pm->width() + 5, yPos, text() );
        }
        // end cut & paste (modulo pixmap centering)
    }

    else {
        // big icons -> draw text below icon
        static const int margin = 3;
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
            int x = (w - fm.width( text() )) / 2;
            x = QMAX( x, margin );
            p->drawText( x, y, text() );
        }
    }

    // draw sunken selection
    if ( isCurrent() || isSelected() ) {
        qDrawShadePanel( p, 1, 0, w -2, height(box),
                         box->colorGroup(), true, 1, 0L );
    }
}

int KURLBarItem::width( const QListBox *lb ) const
{
    int min = 0;
    bool vertical = (static_cast<const KURLBarListBox*>(lb)->orientation()
                     == Qt::Vertical);

    if ( m_parent->iconSize() < KIcon::SizeMedium )
        min = QListBoxPixmap::width( lb );
    else
        min = QMAX(lb->fontMetrics().width( text() ), pixmap()->width()) + 6;

    if ( vertical ) {
        // qDebug("** min: %i, box: %i", min, lb->viewport()->width());
        return QMAX( min, lb->viewport()->width() );
    }
    else
        return min;
}

int KURLBarItem::height( const QListBox *lb ) const
{
    int min = 0;
    bool horiz = (static_cast<const KURLBarListBox*>(lb)->orientation()
                  == Qt::Horizontal);

    if ( m_parent->iconSize() < KIcon::SizeMedium )
        min = QListBoxPixmap::height( lb );
    else
        min = lb->fontMetrics().lineSpacing() + pixmap()->height() + 6;

    if ( horiz )
        return QMAX( min, lb->viewport()->height() );
    else
        return min;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


KURLBar::KURLBar( bool useGlobalItems, QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f ),
      m_activeItem( 0L ),
      m_useGlobal( useGlobalItems ),
      m_listBox( 0L ),
      m_iconSize( KIcon::SizeMedium )
{
    setListBox( 0L );
}

KURLBar::~KURLBar()
{
}

KURLBarItem * KURLBar::insertItem(const KURL& url, const QString& description,
                                  bool applicationLocal,
                                  const QString& icon, int group )
{
    KURLBarItem *item = new KURLBarItem(this, url, description, icon, group);
    item->setApplicationLocal( applicationLocal );
    m_listBox->insertItem( item );
    return item;
}

void KURLBar::setOrientation( Qt::Orientation orient )
{
    m_listBox->setOrientation( orient );
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
        setOrientation( Vertical );
    }
    else {
        m_listBox = view;
        if ( m_listBox->parentWidget() != this )
            m_listBox->reparent( this, QPoint(0,0) );
        m_listBox->resize( width(), height() );
    }

    m_listBox->setSelectionMode( KListBox::Single );
    QPalette pal = palette();
    QColor gray = pal.color( QPalette::Normal, QColorGroup::Mid );
    pal.setColor( QPalette::Normal,   QColorGroup::Base, gray );
    pal.setColor( QPalette::Inactive, QColorGroup::Base, gray );

    setPalette( pal );
    m_listBox->viewport()->setBackgroundMode( PaletteMid );

    connect( m_listBox, SIGNAL( executed( QListBoxItem * ) ),
             SLOT( slotSelected( QListBoxItem * )));
    connect( m_listBox, SIGNAL( dropped( QDropEvent * )),
             this, SLOT( slotDropped( QDropEvent * )));
    connect( m_listBox, SIGNAL( contextMenuRequested( QListBoxItem *,
                                                      const QPoint& )),
             SLOT( slotContextMenuRequested( QListBoxItem *, const QPoint& )));
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

QSize KURLBar::sizeHint() const
{
    int w = 0;
    int h = 0;
    QListBoxItem *item;
    bool vertical = isVertical();

    for ( item = m_listBox->firstItem(); item; item = item->next() ) {
        if ( vertical ) {
            w = QMAX( w, item->width( m_listBox ) );
            h += item->height( m_listBox );
        }
        else {
            w += item->width( m_listBox );
            h = QMAX( h, item->height( m_listBox ) );
        }
    }

    if ( vertical && m_listBox->verticalScrollBar()->isVisible() )
        w += m_listBox->verticalScrollBar()->width();
    else if ( !vertical && m_listBox->horizontalScrollBar()->isVisible() )
        h += m_listBox->horizontalScrollBar()->height();

    if ( w == 0 && h == 0 )
        return QSize( 100, 200 );
    else
        return QSize( 6 + w, h );
}

QSize KURLBar::minimumSizeHint() const
{
    return sizeHint();
}

void KURLBar::slotSelected( QListBoxItem *item )
{
    if ( item && item != m_activeItem ) {
        KURLBarItem *it = static_cast<KURLBarItem*>( item );
        m_activeItem = it;
        emit activated( it->url() );
    }
}

void KURLBar::setCurrentItem( const KURL& url )
{
    QString u = url.url(-1);

    if ( m_activeItem && m_activeItem->url().url(-1) == u )
        return;

    bool hasURL = false;
    QListBoxItem *item = m_listBox->firstItem();
    while ( item ) {
        if ( static_cast<KURLBarItem*>( item )->url().url(-1) == u ) {
            m_listBox->setCurrentItem( item );
            hasURL = true;
            break;
        }
        item = item->next();
    }

    if ( !hasURL ) {
        m_listBox->clearSelection();
        m_activeItem = 0L;
    }
}

KURLBarItem * KURLBar::currentItem() const
{
    QListBoxItem *item = m_listBox->item( m_listBox->currentItem() );
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
    KConfigGroupSaver cs( appConfig, itemGroup );
    m_iconSize = appConfig->readNumEntry( "Speedbar IconSize", m_iconSize );

    if ( m_useGlobal ) { // read global items
        KConfig *globalConfig = KGlobal::config();
        KConfigGroupSaver cs( globalConfig, itemGroup + " (Global)");
        int num = globalConfig->readNumEntry( "Number of Entries" );
        for ( int i = 0; i < num; i++ ) {
            readItem( i, globalConfig, false );
        }
    }

    // read application local items
    int num = appConfig->readNumEntry( "Number of Entries" );
    for ( int i = 0; i < num; i++ ) {
        readItem( i, appConfig, true );
    }
}

void KURLBar::readItem( int i, KConfig *config, bool applicationLocal )
{
    QString number = QString::number( i );

    const KURL& url = config->readEntry( QString("URL_") + number );
    if ( url.isMalformed() || !KProtocolInfo::isKnownProtocol( url ))
        return; // nothing we could do.

    insertItem( url,
                config->readEntry( QString("Description_") + number ),
                applicationLocal,
                config->readEntry( QString("Icon_") + number ),
                config->readNumEntry( QString("IconGroup_") + number ) );
}

void KURLBar::writeConfig( KConfig *config, const QString& itemGroup )
{
    KConfigGroupSaver cs1( config, itemGroup );
    config->writeEntry( "Speedbar IconSize", m_iconSize );

    int i = 0;
    int numLocal = 0;
    KURLBarItem *item = static_cast<KURLBarItem*>( m_listBox->firstItem() );

    while ( item ) {
        if ( item->applicationLocal() ) {
            writeItem( item, numLocal, config, false );
            numLocal++;
        }

        i++;
        item = static_cast<KURLBarItem*>( item->next() );
    }
    config->writeEntry("Number of Entries", numLocal);


    // write the global entries to kdeglobals, if any
    bool haveGlobalEntries = (i > numLocal);
    if ( m_useGlobal && haveGlobalEntries ) {
        config->setGroup( itemGroup + " (Global)" );

        int numGlobals = 0;
        item = static_cast<KURLBarItem*>( m_listBox->firstItem() );

        while ( item ) {
            if ( !item->applicationLocal() ) {
                writeItem( item, numGlobals, config, true );
                numGlobals++;
            }

            item = static_cast<KURLBarItem*>( item->next() );
        }
        config->writeEntry("Number of Entries", numGlobals, true, true);
    }
}

void KURLBar::writeItem( KURLBarItem *item, int i, KConfig *config,
                         bool global )
{
    QString Description = "Description_";
    QString URL = "URL_";
    QString Icon = "Icon_";
    QString IconGroup = "IconGroup_";

    QString number = QString::number( i );
    config->writeEntry( URL + number, item->url().prettyURL(), true, global );
    config->writeEntry( Description + number, item->description(),true,global);
    config->writeEntry( Icon + number, item->icon(), true, global );
    config->writeEntry( IconGroup + number, item->iconGroup(), true, global );
}


void KURLBar::slotDropped( QDropEvent *e )
{
    KURL::List urls;
    if ( KURLDrag::decode( e, urls ) ) {
        KURL url;
        QString description;
        QString icon;
        bool appLocal = false;
        KURLBarItem *item = 0L;

        KURL::List::Iterator it = urls.begin();
        for ( ; it != urls.end(); ++it ) {
            url = *it;
            if ( KURLBarItemDialog::getInformation( m_useGlobal,
                                                    url, description, icon,
                                                    appLocal, this ) ) {
                item = insertItem( url, description, appLocal, icon );
            }
        }
    }
}

void KURLBar::slotContextMenuRequested( QListBoxItem *item, const QPoint& pos )
{
    static const int IconSize   = 10;
    static const int AddItem    = 20;
    static const int EditItem   = 30;
    static const int RemoveItem = 40;

    // also emit activated(), as the item will be painted as "current" anyway
    if ( item )
        slotSelected( item );

    bool smallIcons = m_iconSize < KIcon::SizeMedium;
    QPopupMenu *popup = new QPopupMenu();
    popup->insertItem( smallIcons ?
                       i18n("&Large Icons") : i18n("&Small Icons"),
                       IconSize );
    popup->insertSeparator();
    popup->insertItem( i18n("&Add entry..."), AddItem );
    popup->insertItem( i18n("&Edit entry..."), EditItem );
    popup->insertSeparator();
    popup->insertItem( SmallIcon("editdelete"), i18n("&Remove entry"),
                       RemoveItem );

    popup->setItemEnabled( EditItem, item != 0L );
    popup->setItemEnabled( RemoveItem, item != 0L );

    int result = popup->exec( pos );
    switch ( result ) {
        case IconSize:
            setIconSize( smallIcons ? KIcon::SizeMedium : KIcon::SizeSmall );
            m_listBox->triggerUpdate( true );
            break;
        case AddItem:
            addNewItem();
            break;
        case EditItem:
            editItem( static_cast<KURLBarItem *>( item ) );
            break;
        case RemoveItem:
            delete item;
            break;
        default: // abort
            break;
    }
}

bool KURLBar::addNewItem()
{
    KURL url;
    url.setPath( QDir::homeDirPath() );
    KURLBarItem *item = new KURLBarItem( this, url,
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
    KURL url            = item->url();
    QString description = item->description();
    QString icon        = item->icon();
    bool appLocal       = item->applicationLocal();

    if ( KURLBarItemDialog::getInformation( m_useGlobal,
                                            url, description,
                                            icon, appLocal, this ))
    {
        item->setURL( url );
        item->setDescription( description );
        item->setIcon( icon );
        item->setApplicationLocal( appLocal );
        m_listBox->triggerUpdate( true );
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


KURLBarListBox::KURLBarListBox( QWidget *parent, const char *name )
    : KListBox( parent, name )
{
    m_toolTip = new KURLBarToolTip( this );
    setAcceptDrops( true );
    viewport()->setAcceptDrops( true );
}

KURLBarListBox::~KURLBarListBox()
{
    delete m_toolTip;
}

QDragObject * KURLBarListBox::dragObject()
{
    KURL::List urls;
    KURLBarItem *item = static_cast<KURLBarItem*>( firstItem() );

    while ( item ) {
        if ( item->isSelected() )
            urls.append( item->url() );
        item = static_cast<KURLBarItem*>( item->next() );
    }

    if ( !urls.isEmpty() ) // ### use custom drag-object with description etc.?
        return KURLDrag::newDrag( urls, this, "urlbar drag" );

    return 0L;
}

void KURLBarListBox::contentsDragEnterEvent( QDragEnterEvent *e )
{
    e->accept( KURLDrag::canDecode( e ));
}

void KURLBarListBox::contentsDropEvent( QDropEvent *e )
{
    emit dropped( e );
}

void KURLBarListBox::setOrientation( Qt::Orientation orient )
{
    if ( orient == Vertical ) {
        setColumnMode( 1 );
        setRowMode( Variable );
    }
    else {
        setRowMode( 1 );
        setColumnMode( Variable );
    }

    m_orientation = orient;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


bool KURLBarItemDialog::getInformation( bool allowGlobal, KURL& url,
                                        QString& description, QString& icon,
                                        bool& appLocal, QWidget *parent )
{
    KURLBarItemDialog *dialog = new KURLBarItemDialog( allowGlobal, url,
                                                       description, icon,
                                                       appLocal, parent );
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
                                      QWidget *parent, const char *name )
    : KDialogBase( parent, name, true,
                   i18n("Edit Quick Access entry"), Ok | Cancel, Ok, true )
{
    QVBox *box = new QVBox( this );
    QString text = i18n("<qt><b>Please set url, icon and a description.</b></br></qt>");
    QLabel *label = new QLabel( text, box );
    label->setAlignment( 0 ); // disable word-break

    QGrid *grid = new QGrid( 2, box );
    label = new QLabel( i18n("URL:"), grid );
    m_urlEdit = new KURLRequester( url.prettyURL(), grid );
    m_urlEdit->setMode( KFile::Directory );

    grid->setSpacing( spacingHint() );
    label = new QLabel( i18n("Choose an &icon:"), grid );

    m_iconButton = new KIconButton( grid, "icon button" );
    if ( icon.isEmpty() )
        icon = KMimeType::iconForURL( url );
    m_iconButton->setIcon( icon );
    label->setBuddy( m_iconButton );

    label = new QLabel( i18n("&Description:"), grid );
    m_edit = new KLineEdit( grid, "description edit" );
    m_edit->setText( description.isEmpty() ? url.fileName() : description );
    label->setBuddy( m_edit );

    if ( allowGlobal ) {
        m_appLocal = new QCheckBox(i18n("&Only for this application"), box);
        m_appLocal->setChecked( appLocal );
        QWhatsThis::add( m_appLocal,
                         i18n("Select this setting if you want the\n"
                              "entry only for the current application.\n\n"
                              "Otherwise it will be available in all\n"
                              "in all applications."));
    }
    else
        m_appLocal = 0L;

    m_edit->setFocus();
    setMainWidget( box );
}

KURLBarItemDialog::~KURLBarItemDialog()
{
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


#include "kurlbar.moc"
