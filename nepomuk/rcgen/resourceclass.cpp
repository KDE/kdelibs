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

#include "resourceclass.h"
#include "resourcetemplate.h"

// dummy resource for low level resource inheritance
ResourceClass* ResourceClass::s_defaultResource = new ResourceClass( "http://www.w3.org/2000/01/rdf-schema#Resource" );


static QString writeComment( const QString& comment, int indent )
{
  static const int maxLine = 50;

  QString s;

  if( !comment.isEmpty() ) {
    s += QString().fill( ' ', indent );
    s += "/**\n"
      + QString().fill( ' ', indent+1 )
      + "* ";

    QStringList words = comment.split( QRegExp("\\s") );
    int cnt = 0;
    for( int i = 0; i < words.count(); ++i ) {
      if( cnt >= maxLine ) {
	s += '\n'
	  + QString().fill( ' ', indent+1 )
	  + "* ";
	cnt = 0;
      }

      s += words[i] + ' ';
      cnt += words[i].length();
    }

    if( cnt > 0 )
      s += '\n';
    s += QString().fill( ' ', indent+1 )
      + "*/";
  }

  return s;
}



Property::Property()
  : list(true)
{
}


Property::Property( const QString& uri_,
		    const QString& type_ )
  : uri(uri_),
    type(type_),
    list(true)
{
}


QString Property::name() const
{
  //
  // many predicates are named "hasSomething"
  // we remove the "has" becasue setHasSomething sounds weird
  //
  QString n = uri.section( '#', -1 );
  if( n.toLower().startsWith( "has" ) )
    return n.mid( 3 );
  else
    return n;
}


QString Property::typeString( bool simple ) const
{
  QString t;
  if( type.endsWith( "#Literal" ) ) {
    // FIXME: map to all the simple types somehow
    t = "QString";
  }
  else
    t = type.section( '#', -1 );

  if( !simple && list ) {
    if( t == "QString" )
      return "QStringList";
    else
      return "QList<" + t + ">";
  }
  else
    return t;
}


QString Property::setterDeclaration() const
{
  return QString( "   void set%1%2%3( const %4& value );" )
    .arg( name()[0].toUpper() )
    .arg( name().mid(1) )
    .arg( list ? QString("s") : QString() )
    .arg( typeString() );
}


QString Property::getterDeclaration() const
{
  return QString( "   %1 get%2%3%4() const;" )
    .arg( typeString() )
    .arg( name()[0].toUpper() )
    .arg( name().mid(1) )
    .arg( list ? QString("s") : QString() );
}


QString Property::adderDeclaration() const
{
  return QString( "   void add%1%2( const %3& value );" )
    .arg( name()[0].toUpper() )
    .arg( name().mid(1) )
    .arg( typeString( true ) );
}


QString Property::setterDefinition( const QString& resname ) const
{
  return QString( "void Nepomuk::KMetaData::%1::set%2%3%4( const %5& value )\n"
		  "{\n"
		  "   Variant v;\n"
		  "   v.setValue( value );\n"
		  "   setProperty( \"%6\", v );\n"
		  "}\n" )
    .arg( resname )
    .arg( name()[0].toUpper() )
    .arg( name().mid(1) )
    .arg( list ? QString("s") : QString() )
    .arg( typeString() )
    .arg( uri );
}


QString Property::getterDefinition( const QString& resname ) const
{
  return QString( "%1 Nepomuk::KMetaData::%2::get%4%5%6() const\n"
		  "{\n"
		  "   return getProperty( \"%7\" ).value<%1 >();\n" // The space before the ">" is necessary in case we handle a QList
		  "}\n" )
    .arg( typeString() )
    .arg( resname )
    .arg( name()[0].toUpper() )
    .arg( name().mid(1) )
    .arg( list ? QString("s") : QString() )
    .arg( uri );
}


QString Property::adderDefinition( const QString& resname ) const
{
  return QString( "void Nepomuk::KMetaData::%1::add%2%3( const %5& value )\n"
		  "{\n"
		  "   Variant v = getProperty( \"%6\" );\n"
		  "   if( v.userType() == qMetaTypeId<%5>() ) {\n"
		  "      QList<%5> l( v.value<%5>() );\n"
		  "      v.setValue( l );\n"
		  "   }\n"
		  "   QList<%5> l = v.value<QList<%5> >();\n"
		  "   l.append( value );\n"
		  "   v.setValue( l );\n"
		  "   setProperty( \"%6\", v );\n"
		  "}\n" )
    .arg( resname )
    .arg( name()[0].toUpper() )
    .arg( name().mid(1) )
    .arg( typeString( true ) )
    .arg( uri );
}



ResourceClass::ResourceClass()
  : parent( s_defaultResource )
{
}


ResourceClass::ResourceClass( const QString& uri_ )
  : parent( s_defaultResource ),
    uri( uri_ )
{
}


ResourceClass::~ResourceClass()
{
}


QString ResourceClass::name() const
{
  return uri.section( '#', -1 );
}


QString ResourceClass::headerName() const
{
  return name().toLower() + ".h";
}


QString ResourceClass::sourceName() const
{
  return name().toLower() + ".cpp";
}


bool ResourceClass::writeHeader( QTextStream& stream ) const
{
  QString s = headerTemplate;
  s.replace( "RESOURCECOMMENT", writeComment( comment, 0 ) );
  s.replace( "RESOURCENAMEUPPER", name().toUpper() );
  s.replace( "RESOURCENAME", name() );
  s.replace( "PARENTRESOURCELOWER", parent->name().toLower() );
  s.replace( "PARENTRESOURCE", parent->name() );

  QString methods;
  QTextStream ms( &methods );

  QMapIterator<QString, Property*> it( properties );
  while( it.hasNext() ) {
    it.next();
    const Property* p = it.value();
    ms << writeComment( QString("Get property '%1'. ").arg(p->name()) + p->comment, 3 ) << endl;
    ms << p->getterDeclaration() << endl;
    ms << endl;

    ms << writeComment( QString("Set property '%1'. ").arg(p->name()) + p->comment, 3 ) << endl;
    ms << p->setterDeclaration() << endl;
    ms << endl;

    if( p->list ) {
      ms << writeComment( QString("Add a value to property '%1'. ").arg(p->name()) + p->comment, 3 ) << endl;
      ms << p->adderDeclaration() << endl;
      ms << endl;
    }
  }

  s.replace( "METHODS", methods );

  stream << s;

  return true;
}


bool ResourceClass::writeSource( QTextStream& stream ) const
{
  QString s = sourceTemplate;
  s.replace( "RESOURCENAMELOWER", name().toLower() );
  s.replace( "RESOURCENAME", name() );
  s.replace( "RESOURCETYPEURI", uri );
  s.replace( "PARENTRESOURCE", parent->name() );

  QString methods;
  QTextStream ms( &methods );

  QMapIterator<QString, Property*> it( properties );
  while( it.hasNext() ) {
    it.next();
    const Property* p = it.value();

    if( p->type.isEmpty() ) {
      qDebug() << "(ResourceClass::writeSource) type not defined for property: " << p->name() << endl;
      continue;
    }

    ms << p->getterDefinition( name() ) << endl
       << p->setterDefinition( name() ) << endl;
    if( p->list )
      ms << p->adderDefinition( name() ) << endl;
  }

  s.replace( "METHODS", methods );

  stream << s;

  return true;
}


bool ResourceClass::write( const QString& folder ) const
{
  QFile f( folder + headerName() );
  if( !f.open( QIODevice::WriteOnly ) )
    return false;

  QTextStream s( &f );
  if( !writeHeader( s ) )
    return false;

  f.close();

  f.setFileName( folder + sourceName() );
  if( !f.open( QIODevice::WriteOnly ) )
    return false;

  if( !writeSource( s ) )
    return false;

  return true;
}
