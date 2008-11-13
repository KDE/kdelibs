/*
    Copyright (C) 2005, S.R.Haque <srhaque@iee.org>.
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ktimezonewidget.h"

#include <QtCore/QFile>
#include <QtGui/QPixmap>

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ksystemtimezone.h>
#include <ktimezone.h>

class KTimeZoneWidget::Private
{
public:
    enum Columns
    {
        CityColumn = 0,
        RegionColumn,
        CommentColumn
    };

    enum Roles
    {
        ZoneRole = Qt::UserRole + 0xF3A3CB1
    };
};

#ifndef KDE_USE_FINAL
static bool localeLessThan (const QString &a, const QString &b)
{
    return QString::localeAwareCompare(a, b) < 0;
}
#endif

KTimeZoneWidget::KTimeZoneWidget( QWidget *parent, KTimeZones *db )
  : QTreeWidget( parent ),
    d( 0 )
{
  // If the user did not provide a timezone database, we'll use the system default.
  setRootIsDecorated(false);
  setHeaderLabels( QStringList() << i18nc("Define an area in the time zone, like a town area", "Area" ) << i18nc( "Time zone", "Region" ) << i18n( "Comment" ) );

  // Collect zones by localized city names, so that they can be sorted properly.
  QStringList cities;
  QHash<QString, KTimeZone> zonesByCity;

  if (!db) {
      db = KSystemTimeZones::timeZones();

      // add UTC to the defaults default
      KTimeZone utc = KTimeZone::utc();
      cities.append(utc.name());
      zonesByCity.insert(utc.name(), utc);
  }

  const KTimeZones::ZoneMap zones = db->zones();
  for ( KTimeZones::ZoneMap::ConstIterator it = zones.begin(); it != zones.end(); ++it ) {
    KTimeZone zone = it.value();
    const QString continentCity = displayName( zone );
    int separator = continentCity.lastIndexOf('/');
    QString city = continentCity.right(continentCity.length() - separator - 1)
                   + continentCity.left(separator);

    cities.append( city );
    zonesByCity.insert( city, zone );
  }
  qSort( cities.begin(), cities.end(), localeLessThan );

  foreach ( const QString &city, cities ) {
    KTimeZone zone = zonesByCity[city];
    QString tzName = zone.name();
    QString comment = zone.comment();

    if ( !comment.isEmpty() )
      comment = i18n( comment.toUtf8() );

    // Convert:
    //
    //  "Europe/London", "GB" -> "London", "Europe/GB".
    //  "UTC",           ""   -> "UTC",    "".
    QStringList continentCity = displayName( zone ).split( '/' );

    QTreeWidgetItem *listItem = new QTreeWidgetItem( this );
    listItem->setText( Private::CityColumn, continentCity[ continentCity.count() - 1 ] );
    continentCity[ continentCity.count() - 1 ] = zone.countryCode();

    listItem->setText( Private::RegionColumn, continentCity.join( QChar('/') ) );
    listItem->setText( Private::CommentColumn, comment );
    listItem->setData( Private::CityColumn, Private::ZoneRole, tzName ); // store complete path in custom role

    // Locate the flag from /l10n/%1/flag.png.
    QString flag = KStandardDirs::locate( "locale", QString( "l10n/%1/flag.png" ).arg( zone.countryCode().toLower() ) );
    if ( QFile::exists( flag ) )
      listItem->setIcon( Private::RegionColumn, QPixmap( flag ) );
  }
}

KTimeZoneWidget::~KTimeZoneWidget()
{
  delete d;
}

QString KTimeZoneWidget::displayName( const KTimeZone &zone )
{
  return i18n( zone.name().toUtf8() ).replace( '_', ' ' );
}

QStringList KTimeZoneWidget::selection() const
{
    QStringList selection;

    // Loop through all entries.
    foreach ( QTreeWidgetItem* listItem, selectedItems() )
        selection.append( listItem->data( Private::CityColumn, Private::ZoneRole ).toString() );

    return selection;
}

void KTimeZoneWidget::setSelected( const QString &zone, bool selected )
{
    bool found = false;

    // The code was using findItems( zone, Qt::MatchExactly, Private::ZoneColumn )
    // previously, but the underlying model only has 3 columns, the "hidden" column
    // wasn't available in there.

    // Loop through all entries.
    const int rowCount = model()->rowCount(QModelIndex());
    for (int row = 0; row < rowCount; ++row) {
        const QModelIndex index = model()->index(row, Private::CityColumn );
        const QString tzName = index.data(Private::ZoneRole).toString();
        if (tzName == zone) {
            selectionModel()->select(index, selected ? (QItemSelectionModel::Select | QItemSelectionModel::Rows) : (QItemSelectionModel::Deselect | QItemSelectionModel::Rows));

            // Ensure the selected item is visible as appropriate.
            scrollTo( index );

            found = true;
        }
    }

    if ( !found )
        kDebug() << "No such zone: " << zone;
}

#include "ktimezonewidget.moc"
