/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <dfaure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef K3SQLPROPERTYMAP_H
#define K3SQLPROPERTYMAP_H

#ifndef QT_NO_SQL
#include <Qt3Support/Q3SqlPropertyMap>
#endif

#include <kde3support_export.h>

/**
 * Install a Qt SQL property map with entries for all KDE widgets
 * Call this in any application using KDE widgets in Q3SqlForm or Q3DataView.
 *
 * @deprecated use User in the meta object to determine which Q_PROPERTY to use for any widget.
 *
 * <code>
 *  const QMetaObject *metaObject = widget->metaObject();
 *  for (int i = 0; i < metaObject->propertyCount(); ++i) {
 *    const QMetaProperty metaProperty = metaObject->property(i);
 *    if (metaProperty.isUser()) {
 *      QString propertyToUse = metaProperty.name();
 *      break;
 *    }
 *  }
 * </code>
 */
static inline KDE_DEPRECATED void kInstallKDEPropertyMap()
{
#ifndef QT_NO_SQL
  static bool installed = false;
  if (installed) return;
  installed = true;
  // QSqlPropertyMap takes ownership of the new default map.
  Q3SqlPropertyMap *kdeMap = new Q3SqlPropertyMap;
  kdeMap->insert( QLatin1String("KColorButton"), QLatin1String("color") );
  kdeMap->insert( QLatin1String("KComboBox"), QLatin1String("currentIndex") );
  kdeMap->insert( QLatin1String("KDatePicker"), QLatin1String("date") );
  kdeMap->insert( QLatin1String("KDateWidget"), QLatin1String("date") );
  kdeMap->insert( QLatin1String("KDateTimeWidget"), QLatin1String("dateTime") );
  kdeMap->insert( QLatin1String("KEditListBox"), QLatin1String("items") );
  kdeMap->insert( QLatin1String("KFontCombo"), QLatin1String("family") );
  kdeMap->insert( QLatin1String("KFontRequester"), QLatin1String("font") );
  kdeMap->insert( QLatin1String("KFontChooser"), QLatin1String("font") );
  kdeMap->insert( QLatin1String("KHistoryCombo"), QLatin1String("currentItem") );
  kdeMap->insert( QLatin1String("KListWidget"), QLatin1String("currentItem") );
  kdeMap->insert( QLatin1String("KLineEdit"), QLatin1String("text") );
  kdeMap->insert( QLatin1String("KRestrictedLine"), QLatin1String("text") );
  kdeMap->insert( QLatin1String("KTextBrowser"), QLatin1String("source") );
  kdeMap->insert( QLatin1String("KTextEdit"), QLatin1String("text") );
  kdeMap->insert( QLatin1String("KUrlRequester"), QLatin1String("url") );
  kdeMap->insert( QLatin1String("KPasswordEdit"), QLatin1String("password") );
  kdeMap->insert( QLatin1String("KIntNumInput"), QLatin1String("value") );
  kdeMap->insert( QLatin1String("KIntSpinBox"), QLatin1String("value") );
  kdeMap->insert( QLatin1String("KDoubleNumInput"), QLatin1String("value") );
  Q3SqlPropertyMap::installDefaultMap( kdeMap );
#endif
}

#endif // K3SQLPROPERTYMAP_H
