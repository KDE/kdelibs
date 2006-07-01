/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "idmapper.h"

#include <kstandarddirs.h>
#include <kdebug.h>

#include <QFile>
#include <QTextStream>
#include <QVariant>

namespace KRES {

class IdMapperPrivate
{
  public:
    QMap<QString, QVariant> idMap;
    QMap<QString, QString> fingerprintMap;

    QString path;
    QString identifier;
};

IdMapper::IdMapper()
  : d( new IdMapperPrivate )
{
}

IdMapper::IdMapper( const QString &path, const QString &identifier )
  : d( new IdMapperPrivate )
{
  d->path = path;
  d->identifier = identifier;
}

IdMapper::~IdMapper()
{
  delete d;
}

void IdMapper::setPath( const QString &path )
{
  d->path = path;
}

QString IdMapper::path() const
{
  return d->path;
}

void IdMapper::setIdentifier( const QString &identifier )
{
  d->identifier = identifier;
}

QString IdMapper::identifier() const
{
  return d->identifier;
}

QString IdMapper::filename()
{
  QString file = d->path;
  if ( !file.endsWith( "/" ) ) file += '/';
  file += d->identifier;

  return locateLocal( "data", file );
}

bool IdMapper::load()
{
  QFile file( filename() );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    kError(5800) << "Can't read uid map file '" << filename() << "'" << endl;
    return false;
  }

  clear();

  QTextStream ts(&file);
  QString line;
  while ( !ts.atEnd() ) {
    line = ts.readLine( 1024 );
    QStringList parts = line.split( "\x02\x02", QString::KeepEmptyParts );
    d->idMap.insert( parts[ 0 ], parts[ 1 ] );
    d->fingerprintMap.insert( parts[ 0 ], parts[ 2 ] );
  }

  file.close();

  return true;
}

bool IdMapper::save()
{
  QFile file( filename() );
  if ( !file.open( QIODevice::WriteOnly ) ) {
    kError(5800) << "Can't write uid map file '" << filename() << "'" << endl;
    return false;
  }

  QString content;

  QMap<QString, QVariant>::Iterator it;
  for ( it = d->idMap.begin(); it != d->idMap.end(); ++it ) {
    QString fingerprint;
    if ( d->fingerprintMap.contains( it.key() ) )
      fingerprint = d->fingerprintMap[ it.key() ];
    content += it.key() + "\x02\x02" + it.value().toString() + "\x02\x02" + fingerprint + "\r\n";
  }
  QTextStream ts(&file);
  ts << content;
  file.close();

  return true;
}

void IdMapper::clear()
{
  d->idMap.clear();
  d->fingerprintMap.clear();
}

void IdMapper::setRemoteId( const QString &localId, const QString &remoteId )
{
  d->idMap.insert( localId, remoteId );
}

void IdMapper::removeRemoteId( const QString &remoteId )
{
  QMap<QString, QVariant>::Iterator it;
  for ( it = d->idMap.begin(); it != d->idMap.end(); ++it )
    if ( it.value().toString() == remoteId ) {
      d->idMap.erase( it );
      d->fingerprintMap.remove( it.key() );
      return;
    }
}

QString IdMapper::remoteId( const QString &localId ) const
{
  QMap<QString, QVariant>::ConstIterator it;
  it = d->idMap.find( localId );

  if ( it != d->idMap.end() )
    return it.value().toString();
  else
    return QString();
}

QString IdMapper::localId( const QString &remoteId ) const
{
  QMap<QString, QVariant>::ConstIterator it;
  for ( it = d->idMap.begin(); it != d->idMap.end(); ++it )
    if ( it.value().toString() == remoteId )
      return it.key();

  return QString();
}

QString IdMapper::asString() const
{
  QString content;

  QMap<QString, QVariant>::ConstIterator it;
  for ( it = d->idMap.begin(); it != d->idMap.end(); ++it ) {
    QString fp;
    if ( d->fingerprintMap.contains( it.key() ) )
      fp = d->fingerprintMap[ it.key() ];
    content += it.key() + '\t' + it.value().toString() + '\t' + fp + "\r\n";
  }

  return content;
}

void IdMapper::setFingerprint( const QString &localId, const QString &fingerprint )
{
  d->fingerprintMap.insert( localId, fingerprint );
}

QString IdMapper::fingerprint( const QString &localId ) const
{
  if ( d->fingerprintMap.contains( localId ) )
    return d->fingerprintMap[ localId ];
  else
    return QString();
}

QMap<QString, QString> IdMapper::remoteIdMap() const
{
  QMap<QString, QString> reverseMap;
  QMap<QString, QVariant>::ConstIterator it;
  for ( it = d->idMap.begin(); it != d->idMap.end(); ++it ) {
    reverseMap.insert( it.value().toString(), it.key() );
  }
  return reverseMap;
}

}
