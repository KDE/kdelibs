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
#include <QStyleOptionToolButton>
#include <QStylePainter>

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

#include "auth/kauthaction.h"
#include "auth/kauthactionwatcher.h"

static bool s_useIcons = false;

class KPushButton::KPushButtonPrivate
{
public:
    KPushButtonPrivate(KPushButton *_parent) : parent(_parent), m_dragEnabled( false ), authAction(0)
    {
    }

    KPushButton *parent;

    KGuiItem item;
    KStandardGuiItem::StandardItem itemType;
    QPointer<QMenu> delayedMenu;
    QTimer * delayedMenuTimer;
    bool m_dragEnabled;
    QPoint startPos;
    KAuth::Action *authAction;
    // TODO: Remove whenever QIcon overlays will get fixed
    KIcon oldIcon;

    void slotSettingsChanged( int );
    void slotPressedInternal();
    void slotClickedInternal();
    void authStatusChanged(int status);
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
        const int delay=parent->style()->styleHint(QStyle::SH_ToolButton_PopupDelay, 0, parent);
        delayedMenuTimer->start((delay<=0) ? 150:delay);
    }
}

void KPushButton::KPushButtonPrivate::slotClickedInternal()
{
    if (delayedMenuTimer)
        delayedMenuTimer->stop();

    if (authAction) {
        KAuth::Action::AuthStatus s = authAction->earlyAuthorize();
        switch(s) {
        case KAuth::Action::Denied:
            parent->setEnabled(false);
            break;
        case KAuth::Action::Authorized:
            emit parent->authorized(authAction);
            break;
        default:
            break;
        }
    }
}

void KPushButton::KPushButtonPrivate::slotDelayedMenuTimeout() {
    delayedMenuTimer->stop();
    if (!delayedMenu.isNull()) {
        parent->setMenu(delayedMenu);
        parent->showMenu();
        parent->setMenu(0);
    }
}

void KPushButton::KPushButtonPrivate::authStatusChanged(int status)
{
    KAuth::Action::AuthStatus s = (KAuth::Action::AuthStatus)status;

    switch(s) {
        case KAuth::Action::Authorized:
            parent->setEnabled(true);
            if(!oldIcon.isNull()) {
                parent->setIcon(oldIcon);
                oldIcon = KIcon();
            }
            break;
        case KAuth::Action::AuthRequired:
            parent->setEnabled(true);
            oldIcon = KIcon(parent->icon());
            parent->setIcon(KIcon("dialog-password"));
            break;
        default:
            parent->setEnabled(false);
            if(!oldIcon.isNull()) {
                parent->setIcon(oldIcon);
                oldIcon = KIcon();
            }
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

KAuth::Action *KPushButton::authAction() const
{
    return d->authAction;
}

void KPushButton::setAuthAction(const QString &actionName)
{
    if (actionName.isEmpty()) {
        setAuthAction(0);
    } else {
        setAuthAction(new KAuth::Action(actionName));
    }
}

void KPushButton::setAuthAction(KAuth::Action *action)
{
    if (d->authAction == action) {
        return;
    }

    if (d->authAction) {
        disconnect(d->authAction->watcher(), SIGNAL(statusChanged(int)),
                this, SLOT(authStatusChanged(int)));
        //delete d->authAction;
        d->authAction = 0;
        if (!d->oldIcon.isNull()) {
            setIcon(d->oldIcon);
            d->oldIcon = KIcon();
        }
    }

    if (action != 0) {
        d->authAction = action;
        connect(d->authAction->watcher(), SIGNAL(statusChanged(int)),
                this, SLOT(authStatusChanged(int)));
        d->authStatusChanged(d->authAction->status());
    }
}

QSize KPushButton::sizeHint() const
{
    const bool tempSetMenu = !menu() && d->delayedMenu;
    if (tempSetMenu)
        const_cast<KPushButton *>(this)->setMenu(d->delayedMenu);
    const QSize sz = QPushButton::sizeHint();
    if (tempSetMenu)
        const_cast<KPushButton *>(this)->setMenu(0);
    return sz;
}

void KPushButton::paintEvent( QPaintEvent * )
{
    QStylePainter p(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    
    if (d->delayedMenu)
        option.features |= QStyleOptionButton::HasMenu;

    p.drawControl(QStyle::CE_PushButton, option);
}

#include "kpushbutton.moc"
