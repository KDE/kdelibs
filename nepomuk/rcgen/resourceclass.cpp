/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "resourceclass.h"
#include "resourcetemplate.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtCore/QStringList>


extern bool quiet;


static const QString s_typeComment =
"    // We always store all Resource types as plain Resource objects.\n"
"    // It does not introduce any overhead (due to the implicit sharing of\n"
"    // the data and has the advantage that we can mix setProperty calls\n"
"    // with the special Resource subclass methods.\n"
"    // More importantly Resource loads the data as Resource objects anyway.\n";


static QString writeComment( const QString& comment, int indent )
{
    static const int maxLine = 50;

    QString s;

    if( !comment.isEmpty() ) {
        s += QString().fill( ' ', indent );
        s += "/**\n"
             + QString().fill( ' ', indent+1 )
             + "* ";

        QStringList words = comment.split( QRegExp("\\s"), QString::SkipEmptyParts );
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
    : list(true),
      domain(0),
      inverse(0)
{
}


Property::Property( const QString& uri_,
		    const QString& type_ )
    : uri(uri_),
      type(type_),
      list(true),
      domain(0),
      inverse(0)
{
}


QString Property::name() const
{
    //
    // many predicates are named "hasSomething"
    // we remove the "has" becasue setHasSomething sounds weird
    //
    QString n = uri.section( QRegExp( "[#/:]" ), -1 );
    if( n.toLower().startsWith( "has" ) )
        return n.mid( 3 );
    else
        return n;
}


QString Property::typeString( bool simple, bool withNamespace ) const
{
    QString t;
    if( type.contains( "XMLSchema" ) ) {
        // XML Schema types
        // FIXME: move this map somewhere else
        QHash<QString, QString> xmlSchemaTypes;
        xmlSchemaTypes.insert( "integer", "qint64" );
        xmlSchemaTypes.insert( "nonNegativeInteger", "quint64" );
        xmlSchemaTypes.insert( "nonPositiveInteger", "qint64" );
        xmlSchemaTypes.insert( "negativeInteger", "qint64" );
        xmlSchemaTypes.insert( "positiveInteger", "quint64" );
        xmlSchemaTypes.insert( "long", "qint64" );
        xmlSchemaTypes.insert( "unsignedLong", "quint64" );
        xmlSchemaTypes.insert( "int", "qint32" );
        xmlSchemaTypes.insert( "unsignedInt", "quint32" );
        xmlSchemaTypes.insert( "short", "qint16" );
        xmlSchemaTypes.insert( "unsignedShort", "quint16" );
        xmlSchemaTypes.insert( "byte", "char" );
        xmlSchemaTypes.insert( "unsignedByte", "unsigned char" );
        xmlSchemaTypes.insert( "float", "double" );
        xmlSchemaTypes.insert( "double", "double" );
        xmlSchemaTypes.insert( "boolean", "bool" );
        xmlSchemaTypes.insert( "date", "QDate" );
        xmlSchemaTypes.insert( "time", "QTime" );
        xmlSchemaTypes.insert( "dateTime", "QDateTime" );
        xmlSchemaTypes.insert( "duration", "QDateTime" ); // FIXME
        xmlSchemaTypes.insert( "string", "QString" );
        t = xmlSchemaTypes[type.mid(type.lastIndexOf( "#" ) + 1 )];
    }
    else if( type.endsWith( "#Literal" ) ) {
        t = "QString";
    }
    else {
        t = type.section( QRegExp( "[#/:]" ), -1 );
        if( withNamespace )
            t.prepend( "Nepomuk::" );
    }

    if( !simple && list ) {
        if( t == "QString" )
            return "QStringList";
        else
            return "QList<" + t + '>';
    }

    Q_ASSERT( !t.isEmpty() );

    return t;
}


bool Property::hasSimpleType() const
{
    return ( type.contains( "XMLSchema" ) || type.endsWith( "#Literal" ) );
}


QString Property::setterDeclaration( const ResourceClass* rc, bool withNamespace ) const
{
    return QString( "void %1set%2%3%4( const %5& value )" )
        .arg( withNamespace ? QString("Nepomuk::%1::").arg(rc->name()) : QString() )
        .arg( name()[0].toUpper() )
        .arg( name().mid(1) )
        .arg( list ? (name().endsWith('s') ? QLatin1String("es") : QLatin1String("s") ) : QString() )
        .arg( typeString( false, withNamespace ) );
}


QString Property::getterDeclaration( const ResourceClass* rc, bool withNamespace ) const
{
    return QString( "%1 %2%3%4%5() const" )
        .arg( typeString( false, withNamespace ) )
        .arg( withNamespace ? QString("Nepomuk::%1::").arg(rc->name()) : QString() )
        .arg( name()[0].toLower() )
        .arg( name().mid(1) )
        .arg( list ? (name().endsWith('s') ? QLatin1String("es") : QLatin1String("s") ) : QString() );
}


QString Property::adderDeclaration( const ResourceClass* rc, bool withNamespace ) const
{
    return QString( "void %1add%2%3( const %4& value )" )
        .arg( withNamespace ? QString("Nepomuk::%1::").arg(rc->name()) : QString() )
        .arg( name()[0].toUpper() )
        .arg( name().mid(1) )
        .arg( typeString( true, withNamespace ) );
}


QString Property::reversePropertyGetterDeclaration( const ResourceClass* rc, bool withNamespace ) const
{
    Q_ASSERT( rc );
    Q_ASSERT( domain );
    return QString( "%1 %2%3%4Of() const" )
        .arg( QString("QList<") + domain->name( withNamespace ) + QString(">") )
        .arg( withNamespace ? QString("Nepomuk::%1::").arg(rc->name()) : QString() )
        .arg( name()[0].toLower() )
        .arg( name().mid(1) );
}


QString Property::setterDefinition( const ResourceClass* rc ) const
{
    QString s = setterDeclaration( rc, true ) + '\n';

    if( hasSimpleType() || typeString( true ) == "Resource" || !list ) {
        s += QString("{\n"
                     "    setProperty( QUrl(\"%1\"), Variant( value ) );\n"
                     "}\n" )
             .arg( uri );
    }
    else if( list ) {
        s += QString("{\n"
                     "%1"
                     "    QList<Resource> l;\n"
                     "    for( %2::const_iterator it = value.constBegin();\n"
                     "         it != value.constEnd(); ++it ) {\n"
                     "        l.append( Resource( (*it) ) );\n"
                     "    }\n"
                     "    setProperty( QUrl(\"%3\"), Variant( l ) );\n"
                     "}\n" )
             .arg( s_typeComment )
             .arg( typeString() )
             .arg( uri );
    }
    else {
        s += QString("{\n"
                     "%1"
                     "    setProperty( QUrl(\"%2\"), Variant( Resource( value ) ) );\n"
                     "}\n" )
             .arg( s_typeComment )
             .arg( uri );
    }

    return s;
}


QString Property::typeConversionMethod() const
{
    // for properties with cardinality == 1 we use a little hack since there will always be duplication of
    // data.
    if ( typeString(false) == "QStringList" ) {
        return QLatin1String("toStringList())");
    }
    else if ( typeString(true) == "QString" ) {
        return QLatin1String("toStringList() << QString() ).first()");
    }
    else if ( typeString(true) == "qint32" ) {
        return list ? QLatin1String("toIntList())") : QLatin1String("toIntList() << 0 ).first()");
    }
    else if ( typeString(true) == "quint32" ) {
        return list ? QLatin1String("toUnsignedIntList())") : QLatin1String("toUnsignedIntList() << 0 ).first()");
    }
    else if ( typeString(true) == "qint64" ) {
        return list ? QLatin1String("toInt64List())") : QLatin1String("toInt64List() << 0 ).first()");
    }
    else if ( typeString(true) == "quint64" ) {
        return list ? QLatin1String("toUnsignedInt64List())") : QLatin1String("toUnsignedInt64List() << 0 ).first()");
    }
    else if ( typeString(true) == "bool" ) {
        return list ? QLatin1String("toBoolList())") : QLatin1String("toBoolList() << false ).first()");
    }
    else if ( typeString(true) == "double" ) {
        return list ? QLatin1String("toDoubleList())") : QLatin1String("toDoubleList() << 0.0 ).first()");
    }
    else if ( typeString(true) == "QDateTime" ) {
        return list ? QLatin1String("toDateTimeList())") : QLatin1String("toDateTimeList() << QDateTime() ).first()");
    }
    else if ( typeString(true) == "QDate" ) {
        return list ? QLatin1String("toDateList())") : QLatin1String("toDateList() << QDate() ).first()");
    }
    else if ( typeString(true) == "QTime" ) {
        return list ? QLatin1String("toTimeList())") : QLatin1String("toTimeList() << QTime() ).first()");
    }

    if ( !quiet )
        qDebug() << "Unknown type:" << typeString(true);

    return QString();
}


QString Property::getterDefinition( const ResourceClass* rc ) const
{
    QString s = getterDeclaration( rc, true ) + '\n';

    if( hasSimpleType() ) {
        s += QString( "{\n"
                      "    return ( property( QUrl(\"%1\") ).%2;\n"
                      "}\n" )
             .arg( uri )
             .arg( typeConversionMethod() );
    }
    else if( list ) {
        s += QString("{\n"
                     "%1"
                     "    return convertResourceList<%3>( property( QUrl(\"%2\") ).toResourceList() );\n"
                     "}\n" )
             .arg( s_typeComment )
             .arg( uri )
             .arg( typeString( true ) );
    }
    else {
        s += QString("{\n"
                     "%1"
                     "    return %2( property( QUrl(\"%3\") ).toResource().uri() );\n"
                     "}\n" )
             .arg( s_typeComment )
             .arg( typeString( true ) )
             .arg( uri );
    }

    return s;
}


QString Property::adderDefinition( const ResourceClass* rc ) const
{
    QString s = adderDeclaration( rc, true ) + '\n';

    if( hasSimpleType() ) {
        s += QString( "{\n"
                      "    Variant v = property( QUrl(\"%1\") );\n"
                      "    v.append( value );\n"
                      "    setProperty( QUrl(\"%1\"), v );\n"
                      "}\n" )
             .arg( uri );
    }
    else {
        s += QString( "{\n"
                      "%1"
                      "    Variant v = property( QUrl(\"%2\") );\n"
                      "    v.append( Resource( value ) );\n"
                      "    setProperty( QUrl(\"%2\"), v );\n"
                      "}\n" )
             .arg( s_typeComment )
             .arg( uri );
    }

    return s;
}


QString Property::reversePropertyGetterDefinition( const ResourceClass* rc ) const
{
    QString s = reversePropertyGetterDeclaration( rc, true ) + '\n';

    s += QString( "{\n"
                  "    return convertResourceList<%2>( manager()->allResourcesWithProperty( QUrl(\"%1\"), *this ) );\n"
                  "}\n" )
         .arg( uri )
         .arg( domain->name() );

    return s;
}



ResourceClass::ResourceClass()
    : parent( 0 ),
      generate( false )
{
}


ResourceClass::ResourceClass( const QString& uri_ )
    : parent( 0 ),
      generate( false ),
      uri( uri_ )
{
}


ResourceClass::~ResourceClass()
{
}


QString ResourceClass::name( bool withNamespace ) const
{
    QString s = uri.section( QRegExp( "[#/:]" ), -1 );
    if( withNamespace )
        s.prepend( "Nepomuk::" );
    return s;
}


QString ResourceClass::headerName() const
{
    return name().toLower() + ".h";
}


QString ResourceClass::sourceName() const
{
    return name().toLower() + ".cpp";
}


QString ResourceClass::allResourcesDeclaration( bool withNamespace ) const
{
    return QString( "QList<%1%2> %3all%2s()" )
        .arg( withNamespace ? QString("Nepomuk::") : QString() )
        .arg( name() )
        .arg( withNamespace ? QString("Nepomuk::%1::").arg( name() ) : QString() );
}


QString ResourceClass::allResourcesDefinition() const
{
    return QString( "%1\n"
                    "{\n"
                    "    return Nepomuk::convertResourceList<%3>( ResourceManager::instance()->allResourcesOfType( QUrl(\"%2\") ) );\n"
                    "}\n" )
        .arg( allResourcesDeclaration( true ) )
        .arg( uri )
        .arg( name() );
}


QString ResourceClass::pseudoInheritanceDeclaration( ResourceClass* rc, bool withNamespace ) const
{
    return QString( "%1 %2to%3() const" )
        .arg( rc->name( withNamespace ) )
        .arg( withNamespace ? name( true ) + "::" : QString() )
        .arg( rc->name( false ) );
}


QString ResourceClass::pseudoInheritanceDefinition( ResourceClass* rc ) const
{
    return QString( "%1\n"
                    "{\n"
                    "    return %2( *this );\n"
                    "}\n" )
        .arg( pseudoInheritanceDeclaration( rc, true ) )
        .arg( rc->name( true ) );
}


bool ResourceClass::writeHeader( QTextStream& stream ) const
{
    QString s = headerTemplate;
    if( QFile::exists( headerTemplateFilePath ) ) {
        QFile f( headerTemplateFilePath );
        if( !f.open( QIODevice::ReadOnly ) ) {
            qDebug() << "Failed to open " << headerTemplateFilePath;
            return false;
        }
        s = QTextStream( &f ).readAll();
    }
    s.replace( "NEPOMUK_RESOURCECOMMENT", writeComment( comment, 0 ) );
    s.replace( "NEPOMUK_RESOURCENAMEUPPER", name().toUpper() );
    s.replace( "NEPOMUK_RESOURCENAME", name() );
    s.replace( "NEPOMUK_PARENTRESOURCE", parent->name() );

    // A resource that is not part of the currently generated stuff is supposed
    // to be installed in include/nepomuk
    if ( parent->generateClass() ) {
        s.replace( "NEPOMUK_PARENT_INCLUDE", QString("\"%1.h\"").arg( parent->name().toLower() ) );
    }
    else {
        s.replace( "NEPOMUK_PARENT_INCLUDE", QString("<nepomuk/%1.h>").arg( parent->name().toLower() ) );
    }

    QString methods;
    QTextStream ms( &methods );
    QSet<QString> includes;

    QListIterator<const Property*> it( properties );
    while( it.hasNext() ) {
        const Property* p = it.next();

        if( p->type.isEmpty() ) {
            if ( !quiet )
                qDebug() << "(ResourceClass::writeSource) type not defined for property: " << p->name() << endl;
            continue;
        }

        ms << writeComment( QString("Get property '%1'. ").arg(p->name()) + p->comment, 3*4 ) << endl;
        ms << "            " << p->getterDeclaration( this ) << ";" << endl;
        ms << endl;

        ms << writeComment( QString("Set property '%1'. ").arg(p->name()) + p->comment, 3*4 ) << endl;
        ms << "            " << p->setterDeclaration( this ) << ";" << endl;
        ms << endl;

        if( p->list ) {
            ms << writeComment( QString("Add a value to property '%1'. ").arg(p->name()) + p->comment, 3*4 ) << endl;
            ms << "            " << p->adderDeclaration( this ) << ";" << endl;
            ms << endl;
        }

        ms << writeComment( QString( "\\return The URI of the property '%1'." ).arg( p->name() ), 3*4 ) << endl;
        ms << "            " << "static QUrl " << p->name()[0].toLower() << p->name().mid(1) << "Uri();" << endl;
        ms << endl;

        if( !p->hasSimpleType() )
            includes.insert( p->typeString( true ) );
    }


    it = reverseProperties;
    while( it.hasNext() ) {
        const Property* p = it.next();

        if( p->type.isEmpty() ) {
            if ( !quiet )
                qDebug() << "(ResourceClass::writeSource) type not defined for property: " << p->name() << endl;
            continue;
        }

        if ( p->inverse ) {
            // we already define a reverse property. So leave the generated one out
            continue;
        }

        ms << writeComment( QString("Get all resources that have this resource set as property '%1'. ")
                            .arg(p->name()) + p->comment + QString(" \\sa ResourceManager::allResourcesWithProperty"), 3*4 ) << endl;
        ms << "            " << p->reversePropertyGetterDeclaration( this ) << ";" << endl;
        ms << endl;

        if( !p->hasSimpleType() )
            includes.insert( p->domain->name() );
    }


    //
    // Nepomuk does not support multiple inheritance
    // So we have to use a workaround instead
    //
    if( allParents.count() > 1 ) {
        foreach( ResourceClass* rc, allParents ) {
            // ignore the one we derived from
            if( rc != parent ) {
                ms << writeComment( QString("Nepomuk does not support multiple inheritance. Thus, to access "
                                            "properties from all parent classes helper methods like this are "
                                            "introduced. The object returned represents the exact same resource."), 3*4 ) << endl
                   << "            " << pseudoInheritanceDeclaration( rc, false ) << ";" << endl << endl;

                includes.insert( rc->name() );
            }
        }
    }

    ms << writeComment( QString("Retrieve a list of all available %1 resources. "
                                "This list consists of all resource of type %1 that are stored "
                                "in the local Nepomuk meta data storage and any changes made locally. "
                                "Be aware that in some cases this list can get very big. Then it might "
                                "be better to use libKNep directly.").arg( name() ), 3*4 ) << endl;
    ms << "            static " << allResourcesDeclaration( false ) << ";" << endl;

    QString includeString;
    QSetIterator<QString> includeIt( includes );
    while( includeIt.hasNext() ) {
        includeString += "        class " + includeIt.next() + ";\n";
    }

    s.replace( "NEPOMUK_OTHERCLASSES", includeString );
    s.replace( "NEPOMUK_METHODS", methods );

    stream << s;

    return true;
}


bool ResourceClass::writeSource( QTextStream& stream ) const
{
    QString s = sourceTemplate;
    if( QFile::exists( sourceTemplateFilePath ) ) {
        QFile f( sourceTemplateFilePath );
        if( !f.open( QIODevice::ReadOnly ) ) {
            qDebug() << "Failed to open " << sourceTemplateFilePath;
            return false;
        }
        s = QTextStream( &f ).readAll();
    }
    s.replace( "NEPOMUK_RESOURCENAMELOWER", name().toLower() );
    s.replace( "NEPOMUK_RESOURCENAME", name() );
    s.replace( "NEPOMUK_RESOURCETYPEURI", uri );
    s.replace( "NEPOMUK_PARENTRESOURCE", parent->name() );

    QString methods;
    QStringList includes;
    QTextStream ms( &methods );

    QListIterator<const Property*> it( properties );
    while( it.hasNext() ) {
        const Property* p = it.next();

        if( p->type.isEmpty() ) {
            if ( !quiet )
                qDebug() << "(ResourceClass::writeSource) type not defined for property: " << p->name() << endl;
            continue;
        }

        if ( !p->hasSimpleType() ) {
            includes.append( QString( "#include \"%1.h\"" ).arg( p->typeString( true, false ).toLower() ) );
        }

        ms << p->getterDefinition( this ) << endl
           << p->setterDefinition( this ) << endl;
        if( p->list )
            ms << p->adderDefinition( this ) << endl;

        // write the static method that returns the property's Uri
        ms << "QUrl " << name( true ) << "::" << p->name()[0].toLower() << p->name().mid(1) << "Uri()" << endl
           << "{" << endl
           << "    return QUrl(\"" << p->uri << "\");" << endl
           << "}" << endl << endl;
    }

    it = reverseProperties;
    while( it.hasNext() ) {
        const Property* p = it.next();

        if( p->type.isEmpty() ) {
            if ( !quiet )
                qDebug() << "(ResourceClass::writeSource) type not defined for property: " << p->name() << endl;
            continue;
        }

        if ( p->inverse ) {
            // we already define a reverse property. So leave the generated one out
            continue;
        }

        ms << p->reversePropertyGetterDefinition( this ) << endl;

        includes.append( QString( "#include \"%1\"" ).arg( p->domain->headerName() ) );
    }

    //
    // Nepomuk does not support multiple inheritance
    // So we have to use a workaround instead
    //
    if( allParents.count() > 1 ) {
        foreach( ResourceClass* rc, allParents ) {
            // ignore the one we derived from
            if( rc != parent ) {
                ms << pseudoInheritanceDefinition( rc ) << endl;
                includes.append( QString("#include \"%1.h\"").arg( rc->name().toLower() ) );
            }
        }
    }

    ms << allResourcesDefinition() << endl;

    // HACK: remove duplicates and resource include
    includes = includes.toSet().toList();
    includes.removeAll( "#include \"resource.h\"" );

    s.replace( "NEPOMUK_METHODS", methods );
    s.replace( "NEPOMUK_INCLUDES", includes.join( "\n" ) );

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


bool ResourceClass::generateClass() const
{
    return generate;
}
