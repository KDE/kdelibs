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
#include <kiconloader.h>
#include <kdebug.h>

#include "kguiitem.h"

class KGuiItem::KGuiItemPrivate
{
public:
    KGuiItemPrivate()
    {
        m_enabled = true;
        m_hasIcon = false;
        m_iconLoaded = false;
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
        m_iconLoaded = rhs.m_iconLoaded;
        m_hasIcon = rhs.m_hasIcon;

        return *this;
    }

    QString m_text;
    QString m_toolTip;
    QString m_whatsThis;
    QString m_statusText;
    bool m_enabled : 1;
    
    QString m_iconName;
    QIconSet m_iconSet;
    bool m_hasIcon;
    bool m_iconLoaded;
};


KGuiItem::KGuiItem() {
    d = new KGuiItemPrivate;
}

KGuiItem::KGuiItem( const QString &text, const QIconSet &iconSet,
                    const QString &iconName,
                    const QString &toolTip, const QString &whatsThis )
{
    kdWarning() << "This KGuiItem c'tor is deprecated! Please use the c'tor "
                   "that takes either an iconName or an iconSet, but not "
                   "both!" << endl;
                   
    d = new KGuiItemPrivate;
    d->m_text = text;
    if( iconName.isEmpty() )
    {
        if( !iconSet.isNull() )
            setIconSet( iconSet );
    }
    else
        setIconName( iconName );

    d->m_toolTip =  toolTip;
    d->m_whatsThis = whatsThis;
}

KGuiItem::KGuiItem( const QString &text,    const QString &iconName,
                    const QString &toolTip, const QString &whatsThis )
{
    d = new KGuiItemPrivate;
    d->m_text = text;
    d->m_toolTip =  toolTip;
    d->m_whatsThis = whatsThis;
    setIconName( iconName );
}

KGuiItem::KGuiItem( const QString &text,    const QIconSet &iconSet,
                    const QString &toolTip, const QString &whatsThis )
{
    d = new KGuiItemPrivate;
    d->m_text = text;
    d->m_toolTip =  toolTip;
    d->m_whatsThis = whatsThis;
    setIconSet( iconSet );
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
QString KGuiItem::plainText() const {
  QString stripped( d->m_text );
  int pos;
  while( ( pos = stripped.find( '&' ) ) != -1 )
    stripped.replace( pos, 1, QString::null );

  return stripped;
}

QIconSet KGuiItem::iconSet() const
{
    if( d->m_hasIcon )
    {
        if( !d->m_iconLoaded )
        {
            d->m_iconSet = SmallIconSet( d->m_iconName );
            if( d->m_iconSet.isNull() )
            {
                d->m_hasIcon = false;
                return QIconSet();
            }
            d->m_iconLoaded = true;
        }
        return d->m_iconSet;
    }
    else
        return QIconSet();
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

bool KGuiItem::isEnabled() const
{
    return d->m_enabled;
}

bool KGuiItem::hasIconSet() const
{
    return d->m_hasIcon;
}

void KGuiItem::setText( const QString &text ) {
    d->m_text=text;
}

void KGuiItem::setIconSet( const QIconSet &iconset )
{
    d->m_iconSet = iconset;
    d->m_iconName = QString::null;
    d->m_hasIcon = !iconset.isNull();
    d->m_iconLoaded = true;
}

void KGuiItem::setIconName( const QString &iconName )
{
    d->m_iconName = iconName;
    d->m_iconSet = QIconSet();
    d->m_hasIcon = !iconName.isEmpty();
    d->m_iconLoaded = false;
}

void KGuiItem::setToolTip( const QString &toolTip) {
    d->m_toolTip = toolTip;
}
void KGuiItem::setWhatsThis( const QString &whatsThis ) {
    d->m_whatsThis = whatsThis;
}
void KGuiItem::setEnabled( bool enabled ){
    d->m_enabled = enabled;
}

/* vim: et sw=4
 */
