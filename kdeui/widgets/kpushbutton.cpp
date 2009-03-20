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

#include <QtGui/QDrag>
#include <QtGui/QActionEvent>
#include <QtGui/QMenu>
#include <QtCore/QPointer>
#include <QtGui/QStyle>
#include <QtCore/QTimer>

#include <config.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kguiitem.h>
#include <kicon.h>

static bool s_useIcons = false;

class KPushButton::KPushButtonPrivate
{
public:
    KPushButtonPrivate(KPushButton *_parent) : parent(_parent), m_dragEnabled( false )
    {
    }

    KPushButton *parent;

    KGuiItem item;
    KStandardGuiItem::StandardItem itemType;
    QPointer<QMenu> delayedMenu;
    QTimer * delayedMenuTimer;
    bool m_dragEnabled;
    QPoint startPos;

    void slotSettingsChanged( int );
    void slotPressedInternal();
    void slotClickedInternal();
    void slotDelayedMenuTimeout();
    void readSettings();
};

void KPushButton::KPushButtonPrivate::slotSettingsChanged( int /* category */ )
{
    readSettings();
    parent->setIcon( item.icon() );
}

void KPushButton::KPushButtonPrivate::slotPressedInternal()
{
    if (!delayedMenu.isNull()) {
        if (delayedMenuTimer==0) {
            delayedMenuTimer=new QTimer(parent);
            delayedMenuTimer->setSingleShot(true);
            connect(delayedMenuTimer,SIGNAL(timeout()),parent,SLOT(slotDelayedMenuTimeout()));
        }
        int delay=parent->style()->styleHint(QStyle::SH_ToolButton_PopupDelay, 0, parent);
        delayedMenuTimer->start((delay<=0) ? 150:delay);
    }
}

void KPushButton::KPushButtonPrivate::slotClickedInternal()
{
    if (delayedMenuTimer)
        delayedMenuTimer->stop();
}

void KPushButton::KPushButtonPrivate::slotDelayedMenuTimeout() {
    delayedMenuTimer->stop();
    if (!delayedMenu.isNull()) {
        parent->setMenu(delayedMenu);
        parent->showMenu();
        parent->setMenu(0);
    }
}

void KPushButton::KPushButtonPrivate::readSettings()
{
    s_useIcons = KGlobalSettings::showIconsOnPushButtons();
}



KPushButton::KPushButton( QWidget *parent )
    : QPushButton( parent ), d( new KPushButtonPrivate(this) ) 
{
    init( KGuiItem( "" ) );
}

KPushButton::KPushButton( const QString &text, QWidget *parent )
    : QPushButton( parent ), d( new KPushButtonPrivate(this) )
{
    init( KGuiItem( text ) );
}

KPushButton::KPushButton( const KIcon &icon, const QString &text,
                          QWidget *parent )
    : QPushButton( text, parent ), d( new KPushButtonPrivate(this) )
{
    init( KGuiItem( text, icon ) );
}

KPushButton::KPushButton( const KGuiItem &item, QWidget *parent )
    : QPushButton( parent ), d( new KPushButtonPrivate(this) )
{
    init( item );
}

KPushButton::~KPushButton()
{
    delete d;
}

void KPushButton::init( const KGuiItem &item )
{
    d->item = item;
    d->itemType = (KStandardGuiItem::StandardItem) 0;
    d->delayedMenuTimer=0;

    connect(this,SIGNAL(pressed()), this, SLOT(slotPressedInternal()));
    connect(this,SIGNAL(clicked()), this, SLOT(slotClickedInternal()));
    // call QPushButton's implementation since we don't need to
    // set the GUI items text or check the state of the icon set
    QPushButton::setText( d->item.text() );

    static bool initialized = false;
    if ( !initialized ) {
        d->readSettings();
        initialized = true;
    }

    setIcon( d->item.icon() );

    setToolTip( item.toolTip() );

    setWhatsThis(item.whatsThis());

    connect( KGlobalSettings::self(), SIGNAL( settingsChanged(int) ),
             SLOT( slotSettingsChanged(int) ) );
}

bool KPushButton::isDragEnabled() const
{
    return d->m_dragEnabled;
}

void KPushButton::setGuiItem( const KGuiItem& item )
{
    d->item = item;

    // call QPushButton's implementation since we don't need to
    // set the GUI items text or check the state of the icon set
    QPushButton::setText( d->item.text() );
    setIcon( d->item.icon() );
    setToolTip( d->item.toolTip() );
    setEnabled( d->item.isEnabled() );
    setWhatsThis( d->item.whatsThis() );
}

void KPushButton::setGuiItem( KStandardGuiItem::StandardItem item )
{
    setGuiItem( KStandardGuiItem::guiItem(item) );
    d->itemType = item;
}

KStandardGuiItem::StandardItem KPushButton::guiItem() const
{
    return d->itemType;
}

void KPushButton::setText( const QString &text )
{
    QPushButton::setText(text);

    // we need to re-evaluate the icon set when the text
    // is removed, or when it is supplied
    if (text.isEmpty() != d->item.text().isEmpty())
        setIcon(d->item.icon());

    d->item.setText(text);
}

void KPushButton::setIcon( const KIcon &icon )
{
    d->item.setIcon(icon);

    if ( s_useIcons || text().isEmpty() )
        QPushButton::setIcon( icon );
    else
        QPushButton::setIcon( QIcon() );
}

void KPushButton::setIcon( const QIcon &qicon )
{
    d->item.setIcon(KIcon(qicon));
}

void KPushButton::setDragEnabled( bool enable )
{
    d->m_dragEnabled = enable;
}

void KPushButton::mousePressEvent( QMouseEvent *e )
{
    if ( d->m_dragEnabled )
        d->startPos = e->pos();
    QPushButton::mousePressEvent( e );
}

void KPushButton::mouseMoveEvent( QMouseEvent *e )
{
    if ( !d->m_dragEnabled )
    {
        QPushButton::mouseMoveEvent( e );
        return;
    }

    if ( (e->buttons() & Qt::LeftButton) &&
         (e->pos() - d->startPos).manhattanLength() >
         KGlobalSettings::dndEventDelay() )
    {
        startDrag();
        setDown( false );
    }
}

QDrag * KPushButton::dragObject()
{
    return 0;
}

void KPushButton::startDrag()
{
    QDrag *d = dragObject();
    if ( d )
        d->start();
}

void KPushButton::setDelayedMenu(QMenu *delayedMenu)
{
    d->delayedMenu=delayedMenu;
}

QMenu* KPushButton::delayedMenu()
{
    return d->delayedMenu;
}

#include "kpushbutton.moc"
