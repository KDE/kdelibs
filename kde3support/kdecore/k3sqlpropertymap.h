/* This file is part of the KDE project
   Copyright (C) 2006 KDE team

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

#ifndef QT_NO_SQL
#include <Qt3Support/q3sqlpropertymap.h>
#endif

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
  kdeMap->insert( "KColorButton", "color" );
  kdeMap->insert( "KComboBox", "currentIndex" );
  kdeMap->insert( "KDatePicker", "date" );
  kdeMap->insert( "KDateWidget", "date" );
  kdeMap->insert( "KDateTimeWidget", "dateTime" );
  kdeMap->insert( "KEditListBox", "items" );
  kdeMap->insert( "KFontCombo", "family" );
  kdeMap->insert( "KFontRequester", "font" );
  kdeMap->insert( "KFontChooser", "font" );
  kdeMap->insert( "KHistoryCombo", "currentItem" );
  kdeMap->insert( "KListBox", "currentItem" );
  kdeMap->insert( "KLineEdit", "text" );
  kdeMap->insert( "KRestrictedLine", "text" );
  kdeMap->insert( "KTextBrowser", "source" );
  kdeMap->insert( "KTextEdit", "text" );
  kdeMap->insert( "KUrlRequester", "url" );
  kdeMap->insert( "KPasswordEdit", "password" );
  kdeMap->insert( "KIntNumInput", "value" );
  kdeMap->insert( "KIntSpinBox", "value" );
  kdeMap->insert( "KDoubleNumInput", "value" );
  Q3SqlPropertyMap::installDefaultMap( kdeMap );
#endif
}

