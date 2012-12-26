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

#include <QDrag>
#include <QActionEvent>
#include <QMenu>
#include <QtCore/QPointer>
#include <QStyle>
#include <QtCore/QTimer>
#include <QApplication>

#include <kguiitem.h>

#include "kauthobjectdecorator.h"

class KPushButton::KPushButtonPrivate
{
public:
    KPushButtonPrivate(KPushButton *_parent) : parent(_parent), m_dragEnabled( false ), decorator(0)
    {
    }

    KPushButton *parent;

    KStandardGuiItem::StandardItem itemType;
    QPointer<QMenu> delayedMenu;
    QTimer * delayedMenuTimer;
    bool m_dragEnabled;
    QPoint startPos;
    KAuth::ObjectDecorator *decorator;

    void slotPressedInternal();
    void slotClickedInternal();
    void authStatusChanged(KAuth::Action::AuthStatus status);
    void slotDelayedMenuTimeout();
};

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
}

void KPushButton::KPushButtonPrivate::slotDelayedMenuTimeout() {
    delayedMenuTimer->stop();
    if (!delayedMenu.isNull()) {
        parent->setMenu(delayedMenu);
        parent->showMenu();
        parent->setMenu(0);
    }
}


KPushButton::KPushButton( QWidget *parent )
    : QPushButton( parent ), d( new KPushButtonPrivate(this) )
{
    initWidget( KGuiItem( "" ) );
}

KPushButton::KPushButton( const QString &text, QWidget *parent )
    : QPushButton( parent ), d( new KPushButtonPrivate(this) )
{
    initWidget( KGuiItem( text ) );
}

KPushButton::KPushButton( const QIcon &icon, const QString &text,
                          QWidget *parent )
    : QPushButton( text, parent ), d( new KPushButtonPrivate(this) )
{
    initWidget( KGuiItem( text, icon ) );
}

KPushButton::KPushButton( const KGuiItem &item, QWidget *parent )
    : QPushButton( parent ), d( new KPushButtonPrivate(this) )
{
    initWidget( item );
}

KPushButton::~KPushButton()
{
    delete d;
}

void KPushButton::initWidget( const KGuiItem &item )
{
    d->decorator = new KAuth::ObjectDecorator(this);
    connect(d->decorator, SIGNAL(authorized(KAuth::Action)),
            this, SIGNAL(authorized(KAuth::Action)));

    d->itemType = (KStandardGuiItem::StandardItem) 0;
    d->delayedMenuTimer=0;

    connect(this,SIGNAL(pressed()), this, SLOT(slotPressedInternal()));
    connect(this,SIGNAL(clicked()), this, SLOT(slotClickedInternal()));
    KGuiItem::assign(this, item);
}

bool KPushButton::isDragEnabled() const
{
    return d->m_dragEnabled;
}

void KPushButton::setGuiItem( const KGuiItem& item )
{
    KGuiItem::assign(this, item);
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
}

void KPushButton::setIcon( const QIcon &icon )
{
    const bool useIcons = style()->styleHint(QStyle::SH_DialogButtonBox_ButtonsHaveIcons, 0, this);
    if (useIcons || text().isEmpty())
        QPushButton::setIcon( icon );
    else
        QPushButton::setIcon( QIcon() );
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
         QApplication::startDragDistance() )
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

KAuth::Action KPushButton::authAction() const
{
    return d->decorator->authAction();
}

void KPushButton::setAuthAction(const QString &actionName)
{
    d->decorator->setAuthAction(actionName);
}

void KPushButton::setAuthAction(const KAuth::Action &action)
{
    d->decorator->setAuthAction(action);
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

#include "moc_kpushbutton.cpp"
