/* This file is part of the KDE libraries
    Copyright (C) 2001 Holger Freyther (freyher@yahoo.com)
                  based on ideas from Martijn and Simon
                  many thanks to Simon
		  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qstring.h>
#include <qiconset.h>
#include <qpixmap.h>

#include <assert.h>

#include "kguiitem.h"

class KGuiItem::KGuiItemPrivate{
public:
    KGuiItemPrivate()
    {
        m_enabled=true;
    }
    KGuiItemPrivate( const KGuiItemPrivate &rhs )
    {
        (*this ) = rhs;
    }
    KGuiItemPrivate &operator=( const KGuiItemPrivate &rhs )
    {
        m_text = rhs.m_text;
        m_iconSet = rhs.m_iconSet;
        m_iconName = rhs.m_iconName;
        m_toolTip = rhs.m_toolTip;
        m_whatsThis = rhs.m_whatsThis;
        m_statusText = rhs.m_statusText;
        m_enabled = rhs.m_enabled;
    }

    QString m_text;
    QIconSet m_iconSet;
    QString m_iconName;
    QString m_toolTip;
    QString m_whatsThis;
    QString m_statusText;
    bool m_enabled : 1;
};


KGuiItem::KGuiItem() {
    d = new KGuiItemPrivate;
}
KGuiItem::KGuiItem( const QString &text, const QIconSet &iconSet, const QString &iconName,
                    const QString &toolTip, const QString &whatsThis, 
                    const QString &statusText ) {
    d = new KGuiItemPrivate;
    d->m_text = text;
    d->m_iconSet = iconSet;
    d->m_iconName = iconName;
    d->m_toolTip =  toolTip;
    d->m_whatsThis = whatsThis;
    d->m_statusText = statusText;
}

KGuiItem::KGuiItem( const KGuiItem &rhs )
    : d( 0 )
{
    (*this) = rhs;
}

KGuiItem &KGuiItem::operator=( const KGuiItem &rhs ) {
    if ( d == rhs.d )
        return *this;

    assert( rhs.d );

    delete d;
    d = new KGuiItemPrivate( *rhs.d );

    return *this;
}

KGuiItem::~KGuiItem() {
    delete d;
}

QString KGuiItem::text() const {
    return d->m_text;
}
QIconSet KGuiItem::iconSet() const {
    return d->m_iconSet;
}
QString KGuiItem::iconName() const
{
    return d->m_iconName;
}
QString KGuiItem::toolTip() const {
    return d->m_toolTip;
}
QString KGuiItem::whatsThis() const {
    return d->m_whatsThis;
}
QString KGuiItem::statusText() const {
    return d->m_toolTip;
}
bool KGuiItem::isEnabled( ) const {
    return d->m_enabled;
}
void KGuiItem::setText( const QString &text ) {
    d->m_text=text;
}
void KGuiItem::setIconSet( const QIconSet &iconset ) {
    d->m_iconSet = iconset;
}
void KGuiItem::setIconName( const QString &iconName ) {
    d->m_iconName = iconName;
}
void KGuiItem::setToolTip( const QString &toolTip) {
    d->m_toolTip = toolTip;
}
void KGuiItem::setStatusText( const QString &statustext ) {
    d->m_toolTip = statustext;
}
void KGuiItem::setWhatsThis( const QString &whatsThis ) {
    d->m_whatsThis = whatsThis;
}
void KGuiItem::setEnabled( bool enabled ){
    d->m_enabled = enabled;
}

/* vim: et sw=4
 */
