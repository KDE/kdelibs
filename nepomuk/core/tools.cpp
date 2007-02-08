/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "tools.h"
#include "ontology.h"
#include "resourcemanager.h"

#include <kdebug.h>


QString Nepomuk::KMetaData::defaultGraph()
{
  static QString s = "main";
  return s;
}


QString Nepomuk::KMetaData::typePredicate()
{
  static QString s = Ontology::RDF_NAMESPACE + "type";
  return s;
}


QString Nepomuk::KMetaData::valueToRDFLiteral( const Variant& v )
{
  // FIXME: replace this with the real thing
  return v.toString();
}


template<typename T> QStringList convertList( const QList<T>& vl )
{
  QStringList l;
  QListIterator<T> it( vl );
  while( it.hasNext() )
    l.append( Nepomuk::KMetaData::valueToRDFLiteral( Nepomuk::KMetaData::Variant( it.next() ) ) );
  return l;
}


QStringList Nepomuk::KMetaData::valuesToRDFLiterals( const Variant& v )
{
  if( v.simpleType() == qMetaTypeId<int>() )
    return convertList( v.toIntList() );
  else if( v.simpleType() == qMetaTypeId<double>() )
    return convertList( v.toDoubleList() );
  else if( v.simpleType() == qMetaTypeId<bool>() )
    return convertList( v.toBoolList() );
  else if( v.simpleType() == qMetaTypeId<QDate>() )
    return convertList( v.toDateList() );
  else if( v.simpleType() == qMetaTypeId<QTime>() )
    return convertList( v.toTimeList() );
  else if( v.simpleType() == qMetaTypeId<QDateTime>() )
    return convertList( v.toDateTimeList() );
  else if( v.simpleType() == qMetaTypeId<QUrl>() )
    return convertList( v.toUrlList() );
  else if( v.simpleType() == QVariant::String )
    return v.toStringList();
  else {
    kDebug(300004) << "(Ontology) ERROR: unknown list type: " << v.simpleType() << endl;
    return QStringList();
  }
}


Nepomuk::KMetaData::Variant Nepomuk::KMetaData::RDFLiteralToValue( const QString& s )
{
  // FIXME: replace this with the real thing
  return Variant( s );
}


QString Nepomuk::KMetaData::ensureNamespace( const QString& uri )
{
  QString s(uri);
  // very dumb check for a namespace
  // FIXME: improve this
  if( !uri.contains( "://" ) )
    s.prepend( Nepomuk::KMetaData::ResourceManager::instance()->ontology()->defaultNamespace() + '#' );
  return s;
}
