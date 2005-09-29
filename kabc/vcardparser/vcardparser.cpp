/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <qregexp.h>

#include <kmdcodec.h>

#include "vcardparser.h"

#define FOLD_WIDTH 75

using namespace KABC;

static QString backslash( "\\\\" );
static QString comma( "\\," );
static QString newline( "\\n" );

static void addEscapes( QString &str )
{
  str.replace( '\\', backslash );
  str.replace( ',', comma );
  str.replace( '\n', newline );
}

static void removeEscapes( QString &str )
{
  str.replace( newline, "\n" );
  str.replace( comma, "," );
  str.replace( backslash, "\\" );
}

VCardParser::VCardParser()
{
}

VCardParser::~VCardParser()
{
}

VCard::List VCardParser::parseVCards( const QString& text )
{
  static QRegExp sep( "[\x0d\x0a]" );

  VCard currentVCard;
  VCard::List vCardList;
  QString currentLine;

  const QStringList lines = QStringList::split( sep, text );
  QStringList::ConstIterator it;

  bool inVCard = false;
  QStringList::ConstIterator linesEnd( lines.end() );
  for ( it = lines.begin(); it != linesEnd; ++it ) {

    if ( (*it).isEmpty() ) // empty line
      continue;

    if ( (*it)[ 0 ] == ' ' || (*it)[ 0 ] == '\t' ) { // folded line => append to previous
      currentLine += QString( *it ).remove( 0, 1 );
      continue;
    } else {
      if ( inVCard && !currentLine.isEmpty() ) { // now parse the line
        int colon = currentLine.find( ':' );
        if ( colon == -1 ) { // invalid line
          currentLine = (*it);
          continue;
        }

        VCardLine vCardLine;
        const QString key = currentLine.left( colon ).stripWhiteSpace();
        QString value = currentLine.mid( colon + 1 );

        QStringList params = QStringList::split( ';', key );

        // check for group
        if ( params[0].find( '.' ) != -1 ) {
          const QStringList groupList = QStringList::split( '.', params[0] );
          vCardLine.setGroup( groupList[0] );
          vCardLine.setIdentifier( groupList[1] );
        } else
          vCardLine.setIdentifier( params[0] );

        if ( params.count() > 1 ) { // find all parameters
          QStringList::ConstIterator paramIt = params.begin();
          for ( ++paramIt; paramIt != params.end(); ++paramIt ) {
            QStringList pair = QStringList::split( '=', *paramIt );
            if ( pair.size() == 1 ) {
              // correct the fucking 2.1 'standard'
              if ( pair[0].lower() == "quoted-printable" ) {
                pair[0] = "encoding";
                pair[1] = "quoted-printable";
              } else if ( pair[0].lower() == "base64" ) {
                pair[0] = "encoding";
                pair[1] = "base64";
              } else {
                pair.prepend( "type" );
              }
            }
            // This is pretty much a faster pair[1].contains( ',' )...
            if ( pair[1].find( ',' ) != -1 ) { // parameter in type=x,y,z format
              const QStringList args = QStringList::split( ',', pair[ 1 ] );
              QStringList::ConstIterator argIt;
              for ( argIt = args.begin(); argIt != args.end(); ++argIt )
                vCardLine.addParameter( pair[0].lower(), *argIt );
            } else
              vCardLine.addParameter( pair[0].lower(), pair[1] );
          }
        }

        removeEscapes( value );

        params = vCardLine.parameterList();
        if ( params.findIndex( "encoding" ) != -1 ) { // have to decode the data
          QByteArray input, output;
          input = value.local8Bit();
          if ( vCardLine.parameter( "encoding" ).lower() == "b" ||
               vCardLine.parameter( "encoding" ).lower() == "base64" )
            KCodecs::base64Decode( input, output );
          else if ( vCardLine.parameter( "encoding" ).lower() == "quoted-printable" ) {
            // join any qp-folded lines
            while ( value.at( value.length() - 1 ) == '=' && it != linesEnd ) {
              value = value.remove( value.length() - 1, 1 ) + (*it);
              ++it;
            }
            input = value.local8Bit();
            KCodecs::quotedPrintableDecode( input, output );
          }
          if ( vCardLine.parameter( "charset" ).lower() == "utf-8" ) {
            vCardLine.setValue( QString::fromUtf8( output.data(), output.size() ) );
          } else {
            vCardLine.setValue( output );
          }
        } else if ( vCardLine.parameter( "charset" ).lower() == "utf-8" ) {
          vCardLine.setValue( QString::fromUtf8( value.ascii() ) );
        } else
          vCardLine.setValue( value );

        currentVCard.addLine( vCardLine );
      }

      // we do not save the start and end tag as vcardline
      if ( (*it).lower().startsWith( "begin:vcard" ) ) {
        inVCard = true;
        currentLine.setLength( 0 );
        currentVCard.clear(); // flush vcard
        continue;
      }

      if ( (*it).lower().startsWith( "end:vcard" ) ) {
        inVCard = false;
        vCardList.append( currentVCard );
        currentLine.setLength( 0 );
        currentVCard.clear(); // flush vcard
        continue;
      }

      currentLine = (*it);
    }
  }

  return vCardList;
}

QString VCardParser::createVCards( const VCard::List& list )
{
  QString text;
  QString textLine;
  QString encodingType;
  QStringList idents;
  QStringList params;
  QStringList values;
  QStringList::ConstIterator identIt;
  QStringList::Iterator paramIt;
  QStringList::ConstIterator valueIt;

  VCardLine::List lines;
  VCardLine::List::ConstIterator lineIt;
  VCard::List::ConstIterator cardIt;

  bool hasEncoding;

  text.reserve( list.size() * 300 ); // reserve memory to be more efficient

  // iterate over the cards
  VCard::List::ConstIterator listEnd( list.end() );
  for ( cardIt = list.begin(); cardIt != listEnd; ++cardIt ) {
    text.append( "BEGIN:VCARD\r\n" );

    idents = (*cardIt).identifiers();
    for ( identIt = idents.constBegin(); identIt != idents.constEnd(); ++identIt ) {
      lines = (*cardIt).lines( (*identIt) );

      // iterate over the lines
      for ( lineIt = lines.constBegin(); lineIt != lines.constEnd(); ++lineIt ) {
        if ( !(*lineIt).value().asString().isEmpty() ) {
          if ( (*lineIt).hasGroup() )
            textLine = (*lineIt).group() + "." + (*lineIt).identifier();
          else
            textLine = (*lineIt).identifier();

          params = (*lineIt).parameterList();
          hasEncoding = false;
          if ( params.count() > 0 ) { // we have parameters
            for ( paramIt = params.begin(); paramIt != params.end(); ++paramIt ) {
              if ( (*paramIt) == "encoding" ) {
                hasEncoding = true;
                encodingType = (*lineIt).parameter( "encoding" ).lower();
              }

              values = (*lineIt).parameters( *paramIt );
              for ( valueIt = values.constBegin(); valueIt != values.constEnd(); ++valueIt ) {
                textLine.append( ";" + (*paramIt).upper() );
                if ( !(*valueIt).isEmpty() )
                  textLine.append( "=" + (*valueIt) );
              }
            }
          }

          if ( hasEncoding ) { // have to encode the data
            QByteArray input, output;
            if ( encodingType == "b" ) {
              input = (*lineIt).value().toByteArray();
              KCodecs::base64Encode( input, output );
            } else if ( encodingType == "quoted-printable" ) {
              input = (*lineIt).value().toString().utf8();
              input.resize( input.size() - 1 ); // strip \0
              KCodecs::quotedPrintableEncode( input, output, false );
            }

            QString value( output );
            addEscapes( value );
            textLine.append( ":" + value );
          } else {
            QString value( (*lineIt).value().asString() );
            addEscapes( value );
            textLine.append( ":" + value );
          }

          if ( textLine.length() > FOLD_WIDTH ) { // we have to fold the line
            for ( uint i = 0; i <= ( textLine.length() / FOLD_WIDTH ); ++i )
              text.append( ( i == 0 ? "" : " " ) + textLine.mid( i * FOLD_WIDTH, FOLD_WIDTH ) + "\r\n" );
          } else
            text.append( textLine + "\r\n" );
        }
      }
    }

    text.append( "END:VCARD\r\n" );
    text.append( "\r\n" );
  }

  return text;
}
