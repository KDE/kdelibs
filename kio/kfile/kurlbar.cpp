/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <unistd.h>

#include <qcheckbox.h>
#include <qdrawutil.h>
#include <qfontmetrics.h>
#include <qlabel.h>
#include <qgrid.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qvbox.h>
#include <qwhatsthis.h>

#include <kaboutdata.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kicondialog.h>
#include <kiconloader.h>
#include <kinstance.h>
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
                          const QString& icon, KIcon::Group group )
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

QSize KURLBarItem::sizeHint() const
{
    int wmin = 0;
    int hmin = 0;
    const KURLBarListBox *lb =static_cast<const KURLBarListBox*>(listBox());

    if ( m_parent->iconSize() < KIcon::SizeMedium ) {
        wmin = QListBoxPixmap::width( lb );
        hmin = QListBoxPixmap::height( lb );
    }
    else {
        wmin = QMAX(lb->fontMetrics().width(text()), pixmap()->width()) + 6;
        hmin = lb->fontMetrics().lineSpacing() + pixmap()->height() + 6;
    }

    if ( lb->isVertical() )
        wmin = QMAX( wmin, lb->viewport()->sizeHint().width() );
    else
        hmin = QMAX( hmin, lb->viewport()->sizeHint().height() );

    return QSize( wmin, hmin );
}

int KURLBarItem::width( const QListBox *lb ) const
{
    if ( static_cast<const KURLBarListBox *>( lb )->isVertical() )
        return QMAX( sizeHint().width(), lb->viewport()->width() );
    else
        return sizeHint().width();
}

int KURLBarItem::height( const QListBox *lb ) const
{
    if ( static_cast<const KURLBarListBox *>( lb )->isVertical() )
        return sizeHint().height();
    else
        return QMAX( sizeHint().height(), lb->viewport()->height() );
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


KURLBar::KURLBar( bool useGlobalItems, QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f ),
      m_activeItem( 0L ),
      m_useGlobal( useGlobalItems ),
      m_isModified( false ),
      m_listBox( 0L ),
      m_iconSize( KIcon::SizeMedium )
{
    setListBox( 0L );
    setSizePolicy( QSizePolicy( isVertical() ?
                                QSizePolicy::Maximum :
                                QSizePolicy::Preferred,
                                isVertical() ?
                                QSizePolicy::Preferred :
                                QSizePolicy::Maximum ));
    QWhatsThis::add(this, i18n("<qt>The <b>Quick Access</b> panel provides easy access to commonly used file locations.<p>"
                               "Clicking on one of the shortcut entries will take you to that location.<p>"
                               "By right clicking on an entry you can add, edit and remove shortcuts.</qt>"));
}

KURLBar::~KURLBar()
{
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
    QColor selectedTextColor = pal.color( QPalette::Normal, QColorGroup::BrightText );
    pal.setColor( QPalette::Normal,   QColorGroup::Base, gray );
    pal.setColor( QPalette::Normal,   QColorGroup::HighlightedText, selectedTextColor );
    pal.setColor( QPalette::Inactive, QColorGroup::Base, gray );
    pal.setColor( QPalette::Inactive, QColorGroup::HighlightedText, selectedTextColor );

    setPalette( pal );
    m_listBox->viewport()->setBackgroundMode( PaletteMid );

    connect( m_listBox, SIGNAL( mouseButtonClicked( int, QListBoxItem *, const QPoint & ) ),
             SLOT( slotSelected( int, QListBoxItem * )));
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

QSize KURLBar::sizeHint() const
{
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
}

QSize KURLBar::minimumSizeHint() const
{
    QSize s = sizeHint(); // ###
    int w = s.width()  + m_listBox->verticalScrollBar()->width();
    int h = s.height() + m_listBox->horizontalScrollBar()->height();
    return QSize( w, h );
}

void KURLBar::slotSelected( int button, QListBoxItem *item )
{
    if ( button != Qt::LeftButton )
        return;

    slotSelected( item );
}

void KURLBar::slotSelected( QListBoxItem *item )
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
    QString u = url.url(-1);

    if ( m_activeItem && m_activeItem->url().url(-1) == u )
        return;

    bool hasURL = false;
    QListBoxItem *item = m_listBox->firstItem();
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
        KConfigGroupSaver cs( globalConfig, (QString)(itemGroup +" (Global)"));
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
                static_cast<KIcon::Group>(
                    config->readNumEntry( QString("IconGroup_") + number )) );
}

void KURLBar::writeConfig( KConfig *config, const QString& itemGroup )
{
    KConfigGroupSaver cs1( config, itemGroup );
    config->writeEntry( "Speedbar IconSize", m_iconSize );

    if ( !m_isModified )
        return;

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

    m_isModified = false;
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

        KURL::List::Iterator it = urls.begin();
        for ( ; it != urls.end(); ++it ) {
            url = *it;
            if ( KURLBarItemDialog::getInformation( m_useGlobal,
                                                    url, description, icon,
                                                    appLocal, m_iconSize,
                                                    this ) ) {
                (void) insertItem( url, description, appLocal, icon );
                m_isModified = true;
                updateGeometry();
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

    KURL lastURL = m_activeItem ? m_activeItem->url() : KURL();

    bool smallIcons = m_iconSize < KIcon::SizeMedium;
    QPopupMenu *popup = new QPopupMenu();
    popup->insertItem( smallIcons ?
                       i18n("&Large Icons") : i18n("&Small Icons"),
                       IconSize );
    popup->insertSeparator();
    popup->insertItem(SmallIcon("filenew"), i18n("&Add Entry..."), AddItem);
    popup->insertItem(SmallIcon("edit"), i18n("&Edit Entry..."), EditItem);
    popup->insertSeparator();
    popup->insertItem( SmallIcon("editdelete"), i18n("&Remove Entry"),
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
            m_isModified = true;
            break;
        default: // abort
            break;
    }

    // reset current item
    m_activeItem = 0L;
    setCurrentItem( lastURL );
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
    QVBox *box = new QVBox( this );
    QString text = i18n("<qt><b>Please provide a description, URL and icon for this Quick Access entry.</b></br></qt>");
    QLabel *label = new QLabel( text, box );
    box->setSpacing( spacingHint() );
   
    QGrid *grid = new QGrid( 2, box );
    grid->setSpacing( spacingHint() );
    
    QString whatsThisText = i18n("<qt>This is the text that will appear in the Quick Access panel.<p>"
                                 "The description should consist of one or two words "
                                 "that will help you remember what this entry refers to.</qt>");
    label = new QLabel( i18n("&Description:"), grid );
    m_edit = new KLineEdit( grid, "description edit" );
    m_edit->setText( description.isEmpty() ? url.fileName() : description );
    label->setBuddy( m_edit );
    QWhatsThis::add( label, whatsThisText );
    QWhatsThis::add( m_edit, whatsThisText );
    
    whatsThisText = i18n("<qt>This is the location associated with the entry. Any valid URL may be used. For example:<p>"
                         "%1<br>http://www.kde.org<br>ftp://ftp.kde.org/pub/kde/stable<p>"
                         "By clicking on the button next to the text edit box you can browse to an "
                         "appropriate URL.</qt>").arg(QDir::homeDirPath());
    label = new QLabel( i18n("&URL:"), grid );
    m_urlEdit = new KURLRequester( url.prettyURL(), grid );
    m_urlEdit->setMode( KFile::Directory );
    label->setBuddy( m_urlEdit );
    QWhatsThis::add( label, whatsThisText );
    QWhatsThis::add( m_urlEdit, whatsThisText );
    
    whatsThisText = i18n("<qt>This is the icon that will appear in the Quick Access panel.<p>"
                         "Click on the button to select a different icon.</qt>");
    label = new QLabel( i18n("Choose an &icon:"), grid );
    m_iconButton = new KIconButton( grid, "icon button" );
    m_iconButton->setIconSize( iconSize );
    m_iconButton->setStrictIconSize( true );
    if ( icon.isEmpty() )
        icon = KMimeType::iconForURL( url );
    m_iconButton->setIcon( icon );
    label->setBuddy( m_iconButton );
    QWhatsThis::add( label, whatsThisText );
    QWhatsThis::add( m_iconButton, whatsThisText );

    if ( allowGlobal ) {
        QString appName;
        if ( KGlobal::instance()->aboutData() )
            appName = KGlobal::instance()->aboutData()->programName();
        if ( appName.isEmpty() )
            appName = QString::fromLatin1( KGlobal::instance()->instanceName() );
        m_appLocal = new QCheckBox( i18n("&Only show when using this application (%1)").arg( appName ), box );
        m_appLocal->setChecked( appLocal );
        QWhatsThis::add( m_appLocal,
                         i18n("<qt>Select this setting if you want this "
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
