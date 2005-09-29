/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kpushbutton.h"

#include <qdragobject.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

#include "config.h"

#include <kglobalsettings.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kipc.h> 
#include <kapplication.h>

class KPushButton::KPushButtonPrivate
{
public:
    KGuiItem item;
    KStdGuiItem::StdItem itemType;
};

bool KPushButton::s_useIcons = false;

KPushButton::KPushButton( QWidget *parent, const char *name )
    : QPushButton( parent, name ),
      m_dragEnabled( false )
{
    init( KGuiItem( "" ) );
}

KPushButton::KPushButton( const QString &text, QWidget *parent,
                          const char *name)
    : QPushButton( parent, name ),
      m_dragEnabled( false )
{
    init( KGuiItem( text ) );
}

KPushButton::KPushButton( const QIconSet &icon, const QString &text,
                          QWidget *parent, const char *name )
    : QPushButton( text, parent, name ),
      m_dragEnabled( false )
{
    init( KGuiItem( text, icon ) );
}

KPushButton::KPushButton( const KGuiItem &item, QWidget *parent,
                          const char *name )
    : QPushButton( parent, name ),
      m_dragEnabled( false )
{
    init( item );
}

KPushButton::~KPushButton()
{
    if( d )
    {
        delete d;
        d = 0L;
    }
}

void KPushButton::init( const KGuiItem &item )
{
    d = new KPushButtonPrivate;
    d->item = item;
    d->itemType = (KStdGuiItem::StdItem) 0;

    // call QPushButton's implementation since we don't need to 
    // set the GUI items text or check the state of the icon set
    QPushButton::setText( d->item.text() );

    static bool initialized = false;
    if ( !initialized ) {
        readSettings();
        initialized = true;
    }

    setIconSet( d->item.iconSet() );

    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    QToolTip::add( this, item.toolTip() );

    QWhatsThis::add( this, item.whatsThis() );

    if (kapp)
    {
       connect( kapp, SIGNAL( settingsChanged(int) ),
               SLOT( slotSettingsChanged(int) ) );
       kapp->addKipcEventMask( KIPC::SettingsChanged );
    }
}

void KPushButton::readSettings()
{
    s_useIcons = KGlobalSettings::showIconsOnPushButtons();
}

void KPushButton::setGuiItem( const KGuiItem& item )
{
    d->item = item;

    // call QPushButton's implementation since we don't need to 
    // set the GUI items text or check the state of the icon set
    QPushButton::setText( d->item.text() );
    setIconSet( d->item.iconSet() );
    QWhatsThis::add( this, d->item.whatsThis() );

    // Do not add a tooltip to the button automatically as 99% of the time the
    // tooltip is redundant to the button text and it results in QTipManager
    // invoking an eventHandler on the QApplication which breaks certain apps
    // like KDesktop which are sensitive to such things
//    QToolTip::add( this, d->item.toolTip() );
}

void KPushButton::setGuiItem( KStdGuiItem::StdItem item )
{
	setGuiItem( KStdGuiItem::guiItem(item) );
	d->itemType = item;
}

KStdGuiItem::StdItem KPushButton::guiItem() const
{
	return d->itemType;
}

void KPushButton::setText( const QString &text )
{
    QPushButton::setText(text);

    // we need to re-evaluate the icon set when the text
    // is removed, or when it is supplied
    if (text.isEmpty() != d->item.text().isEmpty())
        setIconSet(d->item.iconSet());

    d->item.setText(text);
}

void KPushButton::setIconSet( const QIconSet &iconSet )
{
    d->item.setIconSet(iconSet);

    if ( s_useIcons || text().isEmpty() )
        QPushButton::setIconSet( iconSet );
    else
        QPushButton::setIconSet( QIconSet() );
}

void KPushButton::slotSettingsChanged( int /* category */ )
{
    readSettings();
    setIconSet( d->item.iconSet() );
}

void KPushButton::setDragEnabled( bool enable )
{
    m_dragEnabled = enable;
}

void KPushButton::mousePressEvent( QMouseEvent *e )
{
    if ( m_dragEnabled )
	startPos = e->pos();
    QPushButton::mousePressEvent( e );
}

void KPushButton::mouseMoveEvent( QMouseEvent *e )
{
    if ( !m_dragEnabled )
    {
        QPushButton::mouseMoveEvent( e );
        return;
    }

    if ( (e->state() & LeftButton) &&
         (e->pos() - startPos).manhattanLength() >
         KGlobalSettings::dndEventDelay() )
    {
        startDrag();
        setDown( false );
    }
}

QDragObject * KPushButton::dragObject()
{
    return 0L;
}

void KPushButton::startDrag()
{
    QDragObject *d = dragObject();
    if ( d )
	d->dragCopy();
}

void KPushButton::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kpushbutton.moc"
