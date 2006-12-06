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

#include "ontology.h"
#include "variant.h"

static const QString NS_RDF = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
static const QString NS_RDFS = "http://www.w3.org/2000/01/rdf-schema#";

QString Nepomuk::KMetaData::Ontology::defaultGraph()
{
  // FIXME: replace this with the real thing
  static QString s = "http://nepomuk.semanticdesktop.org/metadata";
  return s;
}


QString Nepomuk::KMetaData::Ontology::typePredicate()
{
  static QString s = NS_RDF + "type";
  return s;
}


QString Nepomuk::KMetaData::Ontology::valueToRDFLiteral( const Variant& v )
{
  // FIXME: replace this with the real thing
  return v.toString();
}


template<typename T> QStringList convertList( const QList<T>& vl )
{
  QStringList l;
  QListIterator<T> it( vl );
  while( it.hasNext() ) {
    Nepomuk::KMetaData::Variant val;
    val.setValue( it.next() );
    l.append( Nepomuk::KMetaData::Ontology::valueToRDFLiteral( val ) );
  }
  return l;
}


QStringList Nepomuk::KMetaData::Ontology::valuesToRDFLiterals( const Variant& v )
{
  if( v.userType() == qMetaTypeId<QList<int> >() )
    return convertList( v.value<QList<int> >() );
  if( v.userType() == qMetaTypeId<QList<double> >() )
    return convertList( v.value<QList<double> >() );
  if( v.userType() == qMetaTypeId<QList<bool> >() )
    return convertList( v.value<QList<bool> >() );
  if( v.userType() == qMetaTypeId<QList<QDate> >() )
    return convertList( v.value<QList<QDate> >() );
  if( v.userType() == qMetaTypeId<QList<QTime> >() )
    return convertList( v.value<QList<QTime> >() );
  if( v.userType() == qMetaTypeId<QList<QDateTime> >() )
    return convertList( v.value<QList<QDateTime> >() );
  if( v.userType() == qMetaTypeId<QList<QUrl> >() )
    return convertList( v.value<QList<QUrl> >() );
  if( v.type() == QVariant::StringList )
    return v.value<QStringList>();
  else {
    qDebug() << "(Ontology) ERROR: unknown list type: " << v.userType() << endl;
    return QStringList();
  }
}


Nepomuk::KMetaData::Variant Nepomuk::KMetaData::Ontology::RDFLiteralToValue( const QString& s )
{
  // FIXME: replace this with the real thing
  return Variant( s );
}


QString Nepomuk::KMetaData::Ontology::rdfNamespace()
{
  return NS_RDF;
}


QString Nepomuk::KMetaData::Ontology::rdfsNamespace()
{
  return NS_RDFS;
}


QString Nepomuk::KMetaData::Ontology::nrlNamespace()
{
  // FIXME
  return QString();
}
