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

#include <qregexp.h>
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
        m_hasIcon = rhs.m_hasIcon;

        return *this;
    }

    QString m_text;
    QString m_toolTip;
    QString m_whatsThis;
    QString m_statusText;
    QString m_iconName;
    QIconSet m_iconSet;
    bool m_hasIcon : 1;
    bool m_enabled : 1;
};


KGuiItem::KGuiItem() {
    d = new KGuiItemPrivate;
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
    int len = d->m_text.length();

    if (len == 0)
        return d->m_text;

    //Can assume len >= 1 from now on.
    QString stripped;

    int resultLength = 0;
    stripped.setLength(len);

    const QChar* data    = d->m_text.unicode();
    for (int pos = 0; pos < len; pos++)
    {
#if QT_VERSION >= 0x030200
        if (data[pos] != '&')
            stripped[resultLength++] = data[pos];
        else if (pos+1<len && data[pos+1]=='&')
            stripped[resultLength++] = data[pos++];
#else
        //We pass through any non-ampersand character,
        //and any ampersand that's preceded by an ampersand
        if (data[pos] != '&' || (pos >= 1 && data[pos-1] == '&') )
            stripped[resultLength++] = data[pos];
#endif
    }

    stripped.truncate(resultLength);

    return stripped;
}

QIconSet KGuiItem::iconSet( KIcon::Group group, int size, KInstance* instance ) const
{
    if( d->m_hasIcon )
    {
        if( !d->m_iconName.isEmpty())
        {
// some caching here would(?) come handy
            return instance->iconLoader()->loadIconSet( d->m_iconName, group, size );
// here is a little problem that with delayed icon loading
// we can't check if the icon really exists ... so what ...
//            if( set.isNull() )
//            {
//                d->m_hasIcon = false;
//                return QIconSet();
//            }
//            return set;
        }
        else
        {
            return d->m_iconSet;
        }
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

bool KGuiItem::hasIcon() const
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
}

void KGuiItem::setIconName( const QString &iconName )
{
    d->m_iconName = iconName;
    d->m_iconSet = QIconSet();
    d->m_hasIcon = !iconName.isEmpty();
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
