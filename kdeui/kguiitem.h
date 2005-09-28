/* This file is part of the KDE libraries
    Copyright (C) 2001 Holger Freyther (freyher@yahoo.com)
                  based on ideas from Martijn and Simon

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.

    Many thanks to Simon tronical Hausmann
*/

#ifndef __kguiitem_h__
#define __kguiitem_h__

#include <qstring.h>
#include <qiconset.h>
#include <qpixmap.h>
#include <qvaluelist.h>
#include <kicontheme.h>
#include <kglobal.h>

/**
 * @short An abstract class for GUI data such as ToolTip and Icon.
 *
 * @author Holger Freyther <freyher@yahoo.com>
 */
class KDEUI_EXPORT KGuiItem
{
public:
    KGuiItem();

    // ### This should probably be explicit in KDE 4; it's easy to get
    // subtle bugs otherwise - the icon name, tooltip and whatsthis text
    // get changed behind your back if you do 'setButtonFoo( "Bar" );'
    // It gives the wrong impression that you just change the text.
    KGuiItem( const QString &text, 
              const QString &iconName  = QString::null,
              const QString &toolTip   = QString::null, 
              const QString &whatsThis = QString::null );

    KGuiItem( const QString &text, const QIconSet &iconSet, 
              const QString &toolTip   = QString::null, 
              const QString &whatsThis = QString::null );

    KGuiItem( const KGuiItem &rhs );
    KGuiItem &operator=( const KGuiItem &rhs );

    ~KGuiItem();

    QString text() const;
    QString plainText() const;
#ifndef KDE_NO_COMPAT
    QIconSet iconSet( KIcon::Group, int size = 0, KInstance* instance = KGlobal::instance()) const;
    QIconSet iconSet() const { return iconSet( KIcon::Small ); }
#else
    QIconSet iconSet( KIcon::Group=KIcon::Small, int size = 0, KInstance* instance = KGlobal::instance()) const;
#endif

    QString iconName() const;
    QString toolTip() const;
    QString whatsThis() const;
    bool isEnabled() const;
    /**
     * returns whether an icon is defined, doesn't tell if it really exists
     */
    bool hasIcon() const;
#ifndef KDE_NO_COMPAT
    bool hasIconSet() const { return hasIcon(); }
#endif

    void setText( const QString &text );
    void setIconSet( const QIconSet &iconset );
    void setIconName( const QString &iconName );
    void setToolTip( const QString &tooltip );
    void setWhatsThis( const QString &whatsThis );
    void setEnabled( bool enable );

private:
    class KGuiItemPrivate;
    KGuiItemPrivate *d;
};

/* vim: et sw=4
 */

#endif

