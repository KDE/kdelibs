/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008-2009 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "queryserializer.h"

#include "filequery.h"
#include "term.h"
#include "literalterm.h"
#include "resourceterm.h"
#include "andterm.h"
#include "orterm.h"
#include "negationterm.h"
#include "comparisonterm.h"
#include "comparisonterm_p.h"
#include "resourcetypeterm.h"
#include "optionalterm.h"

#include "property.h"
#include "class.h"

#include <Soprano/LiteralValue>

#include <kurl.h>
#include <kdebug.h>

#include <QtCore/QXmlStreamWriter>


using namespace Nepomuk::Query;

namespace {

    QString aggregateToString( Nepomuk::Query::ComparisonTerm::AggregateFunction f )
    {
        switch( f ) {
        case Nepomuk::Query::ComparisonTerm::Count:
            return QString::fromLatin1("count");
        case Nepomuk::Query::ComparisonTerm::DistinctCount:
            return QString::fromLatin1("distinctcount");
        case Nepomuk::Query::ComparisonTerm::Max:
            return QString::fromLatin1("max");
        case Nepomuk::Query::ComparisonTerm::Min:
            return QString::fromLatin1("min");
        case Nepomuk::Query::ComparisonTerm::Sum:
            return QString::fromLatin1("sum");
        case Nepomuk::Query::ComparisonTerm::DistinctSum:
            return QString::fromLatin1("distinctsum");
        case Nepomuk::Query::ComparisonTerm::Average:
            return QString::fromLatin1("avg");
        case Nepomuk::Query::ComparisonTerm::DistinctAverage:
            return QString::fromLatin1("distinctavg");
        default:
            return QString();
        }
    }

    Nepomuk::Query::ComparisonTerm::AggregateFunction stringToAggregate( const QStringRef& f )
    {
        if( f == QString::fromLatin1("count") )
            return Nepomuk::Query::ComparisonTerm::Count;
        else if( f == QString::fromLatin1("distinctcount") )
            return Nepomuk::Query::ComparisonTerm::DistinctCount;
        else if( f == QString::fromLatin1("max") )
            return Nepomuk::Query::ComparisonTerm::Max;
        else if( f == QString::fromLatin1("min") )
            return Nepomuk::Query::ComparisonTerm::Min;
        else if( f == QString::fromLatin1("sum") )
            return Nepomuk::Query::ComparisonTerm::Sum;
        else if( f == QString::fromLatin1("distinctsum") )
            return Nepomuk::Query::ComparisonTerm::DistinctSum;
        else if( f == QString::fromLatin1("avg") )
            return Nepomuk::Query::ComparisonTerm::Average;
        else if( f == QString::fromLatin1("distinctavg") )
            return Nepomuk::Query::ComparisonTerm::DistinctAverage;
        else
            return Nepomuk::Query::ComparisonTerm::NoAggregateFunction;
    }


    bool doSerializeTerm( QXmlStreamWriter& xml, const Nepomuk::Query::Term& term )
    {
        switch(term.type()) {
        case Term::Literal: {
            xml.writeStartElement( QLatin1String("literal") );
            const Soprano::LiteralValue value = term.toLiteralTerm().value();
            if( value.isPlain() )
                xml.writeAttribute( QLatin1String("lang"), value.language().toString() );
            else
                xml.writeAttribute( QLatin1String("datatype"), KUrl(value.dataTypeUri()).url() );
            xml.writeCharacters( value.toString() );
            xml.writeEndElement();
            break;
        }

        case Term::Resource:
            xml.writeStartElement( QLatin1String("resource") );
            xml.writeAttribute( QLatin1String("uri"), KUrl( term.toResourceTerm().resource().resourceUri() ).url() );
            xml.writeEndElement();
            break;

        case Term::And:
            xml.writeStartElement( QLatin1String("and") );
            Q_FOREACH( const Term& term, term.toAndTerm().subTerms() ) {
                doSerializeTerm( xml, term );
            }
            xml.writeEndElement();
            break;

        case Term::Or:
            xml.writeStartElement( QLatin1String("or") );
            Q_FOREACH( const Term& term, term.toOrTerm().subTerms() ) {
                doSerializeTerm( xml, term );
            }
            xml.writeEndElement();
            break;

        case Term::Comparison: {
            ComparisonTerm cTerm( term.toComparisonTerm() );

            xml.writeStartElement( QLatin1String("comparison") );

            if( cTerm.property().isValid() )
                xml.writeAttribute( QLatin1String("property"), KUrl(cTerm.property().uri()).url() );
            xml.writeAttribute( QLatin1String("comparator"), Nepomuk::Query::comparatorToString(cTerm.comparator()) );
            if( !cTerm.variableName().isEmpty() )
                xml.writeAttribute( QLatin1String("varname"), cTerm.variableName() );
            if( cTerm.aggregateFunction() != ComparisonTerm::NoAggregateFunction )
                xml.writeAttribute( QLatin1String("aggregate"), aggregateToString(cTerm.aggregateFunction()) );
            if( cTerm.sortWeight() != 0 ) {
                xml.writeAttribute( QLatin1String("sortWeight"), QString::number(cTerm.sortWeight()) );
                xml.writeAttribute( QLatin1String("sortOrder"), cTerm.sortOrder() == Qt::AscendingOrder ? QLatin1String("asc") : QLatin1String("desc") );
            }
            xml.writeAttribute( QLatin1String("inverted"), cTerm.isInverted() ? QLatin1String("true") : QLatin1String("false") );

            doSerializeTerm( xml, cTerm.subTerm() );

            xml.writeEndElement();
            break;
        }

        case Term::ResourceType:
            xml.writeStartElement( QLatin1String("type") );
            xml.writeAttribute( QLatin1String("uri"), KUrl( term.toResourceTypeTerm().type().uri() ).url() );
            xml.writeEndElement();
            break;

        case Term::Negation:
            xml.writeStartElement( QLatin1String("not") );
            doSerializeTerm( xml, term.toNegationTerm().subTerm() );
            xml.writeEndElement();
            break;

        case Term::Optional:
            xml.writeStartElement( QLatin1String("optional") );
            doSerializeTerm( xml, term.toOptionalTerm().subTerm() );
            xml.writeEndElement();
            break;

        default:
            return false;
        }

        return true;
    }


    Term doParseTerm( QXmlStreamReader& xml, bool* ok = 0 )
    {
        Q_ASSERT( xml.isStartElement() );

        if( xml.name() == QLatin1String("literal") ) {
            if( ok )
                *ok = true;
            QXmlStreamAttributes attr = xml.attributes();
            LiteralTerm term;
            if( attr.hasAttribute( QLatin1String("datatype") ) )
                term = LiteralTerm( Soprano::LiteralValue::fromString( xml.readElementText(), KUrl(attr.value( QLatin1String("datatype") ).toString()) ) );
            else
                term = LiteralTerm( Soprano::LiteralValue::createPlainLiteral( xml.readElementText(), attr.value( QLatin1String("lang") ).toString()) );
            // no need to skip to next element due to usage of readElementText?
            return term;
        }

        else if( xml.name() == QLatin1String("resource") ) {
            if( ok )
                *ok = true;
            ResourceTerm term( KUrl(xml.attributes().value( QLatin1String("uri") ).toString()) );
            xml.readNextStartElement(); // skip to next element
            return term;
        }

        else if( xml.name() == QLatin1String("and") ) {
            AndTerm term;
            while( xml.readNextStartElement() ) {
                term.addSubTerm( doParseTerm( xml,  ok ) );
                if( ok && !*ok )
                    return Term();
            }
            return term;
        }

        else if( xml.name() == QLatin1String("or") ) {
            OrTerm term;
            while( xml.readNextStartElement() ) {
                term.addSubTerm( doParseTerm( xml,  ok ) );
                if( ok && !*ok )
                    return Term();
            }
            return term;
        }

        else if( xml.name() == QLatin1String("not") ) {
            if( xml.readNextStartElement() ) {
                Term term = doParseTerm( xml, ok );
                if( ok && !*ok )
                    return Term();
                xml.readNextStartElement(); // skip to next element
                return NegationTerm::negateTerm(term);
            }
            else {
                if( ok )
                    *ok = false;
                return Term();
            }
        }

        else if( xml.name() == QLatin1String("optional") ) {
            if( xml.readNextStartElement() ) {
                Term term = doParseTerm( xml, ok );
                if( ok && !*ok )
                    return Term();
                xml.readNextStartElement(); // skip to next element
                return OptionalTerm::optionalizeTerm(term);
            }
            else {
                if( ok )
                    *ok = false;
                return Term();
            }
        }

        else if( xml.name() == QLatin1String("type") ) {
            if( ok )
                *ok = true;
            ResourceTypeTerm term( KUrl(xml.attributes().value( QLatin1String("uri") ).toString()) );
            xml.readNextStartElement(); // skip to next element
            return term;
        }

        else if( xml.name() == QLatin1String("comparison") ) {
            ComparisonTerm cTerm;

            QXmlStreamAttributes attr = xml.attributes();

            if( attr.hasAttribute( QLatin1String("property") ) )
                cTerm.setProperty( KUrl(attr.value( QLatin1String("property")).toString()) );

            if( attr.hasAttribute( QLatin1String("comparator") ) )
                cTerm.setComparator( Nepomuk::Query::stringToComparator(attr.value( QLatin1String("comparator"))) );

            if( attr.hasAttribute( QLatin1String("varname") ) )
                cTerm.setVariableName( attr.value( QLatin1String("varname")).toString() );

            if( attr.hasAttribute( QLatin1String("aggregate") ) )
                cTerm.setAggregateFunction( stringToAggregate(attr.value( QLatin1String("aggregate"))) );

            if( attr.hasAttribute( QLatin1String("sortWeight") ) )
                cTerm.setSortWeight( attr.value( QLatin1String("sortWeight")).toString().toInt(),
                                     attr.value( QLatin1String("sortOrder")) == QLatin1String("desc") ? Qt::DescendingOrder : Qt::AscendingOrder );

            if( attr.hasAttribute( QLatin1String("inverted") ) )
                cTerm.setInverted( attr.value( QLatin1String("inverted")) == QLatin1String("true") );

            if( xml.readNextStartElement() ) {
                Term term = doParseTerm( xml, ok );
                if( ok && !*ok )
                    return Term();
                cTerm.setSubTerm(term);
                xml.readNextStartElement(); // skip to next element
            }

            if( ok )
                *ok = true;
            return cTerm;
        }

        else {
            kDebug() << "Unknown term type" << xml.name();
            if( ok )
                *ok = false;
            return Term();
        }
    }


    void readQueryAttributes( const QXmlStreamAttributes& attributes, Query& query )
    {
        if( attributes.hasAttribute( QLatin1String("limit") ) )
            query.setLimit( attributes.value( QLatin1String("limit") ).toString().toInt() );
        if( attributes.hasAttribute( QLatin1String("offset") ) )
            query.setOffset( attributes.value( QLatin1String("offset") ).toString().toInt() );
    }
}


QString Nepomuk::Query::serializeQuery( const Query& query )
{
    QString s;
    QXmlStreamWriter xml( &s );

    xml.writeStartDocument();

    if( query.isFileQuery() ) {
        FileQuery fq( query );
        xml.writeStartElement( QLatin1String("filequery") );
        xml.writeAttribute( QLatin1String("queryFiles"), fq.fileMode()&FileQuery::QueryFiles ? QLatin1String("true") : QLatin1String("false") );
        xml.writeAttribute( QLatin1String("queryFolders"), fq.fileMode()&FileQuery::QueryFolders ? QLatin1String("true") : QLatin1String("false") );
    }
    else {
        xml.writeStartElement( QLatin1String("query") );
    }

    xml.writeAttribute( QLatin1String("limit"), QString::number(query.limit()) );
    xml.writeAttribute( QLatin1String("offset"), QString::number(query.offset()) );

    Q_FOREACH( const Query::RequestProperty& rp, query.requestProperties() ) {
        xml.writeStartElement( QLatin1String("requestProperty") );
        xml.writeAttribute( QLatin1String("uri"), KUrl(rp.property().uri()).url() );
        xml.writeAttribute( QLatin1String("optional"), rp.optional() ? QLatin1String("true") : QLatin1String("false") );
        xml.writeEndElement();
    }

    if( query.isFileQuery() ) {
        FileQuery fq( query );
        Q_FOREACH( const KUrl& url, fq.includeFolders() ) {
            xml.writeStartElement( QLatin1String("folder") );
            xml.writeAttribute( QLatin1String("url"), url.url() );
            xml.writeAttribute( QLatin1String("include"), QLatin1String("true") );
            xml.writeEndElement();
        }
        Q_FOREACH( const KUrl& url, fq.excludeFolders() ) {
            xml.writeStartElement( QLatin1String("folder") );
            xml.writeAttribute( QLatin1String("url"), url.url() );
            xml.writeAttribute( QLatin1String("include"), QLatin1String("false") );
            xml.writeEndElement();
        }
    }

    doSerializeTerm( xml, query.term() );

    // close query element
    xml.writeEndElement();
    xml.writeEndDocument();

    return s;
}


Query Nepomuk::Query::parseQuery( const QString& s )
{
    QXmlStreamReader xml( s );

    Query query;

    while( xml.readNextStartElement() ) {
        if( xml.name() == QLatin1String("query") ) {
            readQueryAttributes( xml.attributes(), query );
        }
        else if( xml.name() == QLatin1String("filequery") ) {
            readQueryAttributes( xml.attributes(), query );

            FileQuery fileQuery(query);
            FileQuery::FileMode mode;
            if( xml.attributes().value(QLatin1String("queryFiles") ) == QLatin1String("true") )
                mode |= FileQuery::QueryFiles;
            if( xml.attributes().value(QLatin1String("queryFolders") ) == QLatin1String("true") )
                mode |= FileQuery::QueryFolders;
            fileQuery.setFileMode(mode);

            query = fileQuery;
        }
        else if( xml.name() == QLatin1String("requestProperty") ) {
            query.addRequestProperty( Query::RequestProperty( KUrl(xml.attributes().value( QLatin1String("uri") ).toString()),
                                                              xml.attributes().value( QLatin1String("optional") ) == QLatin1String("true") ) );
            xml.readNextStartElement(); // skip to next element
        }
        else if( xml.name() == QLatin1String("folder") ) {
            if( !query.isFileQuery() ) {
                kDebug() << "Folder tokens are only allowed in filequerys";
                query = Query();
                break;
            }

            FileQuery fileQuery(query);
            if( xml.attributes().value( QLatin1String("include") ) == QLatin1String("true") )
                fileQuery.addIncludeFolder( KUrl( xml.attributes().value( QLatin1String("url") ).toString() ) );
            else
                fileQuery.addExcludeFolder( KUrl( xml.attributes().value( QLatin1String("url") ).toString() ) );
            query = fileQuery;
            xml.readNextStartElement(); // skip to next element
        }
        else {
            bool termOk = false;
            query.setTerm( doParseTerm( xml, &termOk ) );
            if( !termOk ) {
                break;
            }
        }
    }

    return query;
}


QString Nepomuk::Query::serializeTerm( const Term& term )
{
    QString s;
    QXmlStreamWriter xml( &s );

    // FIXME: should we really add the document details?
    xml.writeStartDocument();
    doSerializeTerm( xml, term );
    xml.writeEndDocument();

    return s;
}


Term Nepomuk::Query::parseTerm( const QString& s )
{
    QXmlStreamReader xml( s );
    if( xml.readNextStartElement() )
        return doParseTerm( xml );
    else
        return Term();

}
