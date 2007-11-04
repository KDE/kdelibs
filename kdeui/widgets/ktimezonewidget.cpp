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
      CommentColumn,
      ZoneColumn
    };
};

KTimeZoneWidget::KTimeZoneWidget( QWidget *parent, KTimeZones *db )
  : QTreeWidget( parent ),
    d( 0 )
{
  // If the user did not provide a timezone database, we'll use the system default.
  if ( !db )
      db = KSystemTimeZones::timeZones();

  setRootIsDecorated(false);
  setHeaderLabels( QStringList() << i18n( "Area" ) << i18n( "Region" ) << i18n( "Comment" ) );

  const KTimeZones::ZoneMap zones = db->zones();
  for ( KTimeZones::ZoneMap::ConstIterator it = zones.begin(); it != zones.end(); ++it ) {
    KTimeZone zone = it.value();
    QString tzName = zone.name();
    QString comment = zone.comment();

    if ( !comment.isEmpty() )
      comment = i18n( comment.toUtf8() );

    // Convert:
    //
    //  "Europe/London", "GB" -> "London", "Europe/GB".
    //  "UTC",           ""   -> "UTC",    "".
    QStringList continentCity = displayName( zone ).split( "/" );

    QTreeWidgetItem *listItem = new QTreeWidgetItem( this );
    listItem->setText( Private::CityColumn, continentCity[ continentCity.count() - 1 ] );
    continentCity[ continentCity.count() - 1 ] = zone.countryCode();

    listItem->setText( Private::RegionColumn, continentCity.join( "/" ) );
    listItem->setText( Private::CommentColumn, comment );
    listItem->setText( Private::ZoneColumn, tzName ); /* store complete path in ListView */

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
  return i18n( zone.name().toUtf8() ).replace( "_", " " );
}

QStringList KTimeZoneWidget::selection() const
{
  QStringList selection;

  // Loop through all entries.
  foreach ( QTreeWidgetItem* listItem, selectedItems() )
    selection.append( listItem->text( Private::ZoneColumn ) );

  return selection;
}

void KTimeZoneWidget::setSelected( const QString &zone, bool selected )
{
  bool found = false;

  // Loop through all entries.
  foreach ( QTreeWidgetItem* listItem, findItems( zone, Qt::MatchExactly, Private::ZoneColumn ) ) {
    listItem->setSelected( selected );

    // Ensure the selected item is visible as appropriate.
    scrollTo( indexFromItem( listItem ) );

    found = true;
  }

  if ( !found )
    kDebug() << "No such zone: " << zone;
}

#include "ktimezonewidget.moc"
