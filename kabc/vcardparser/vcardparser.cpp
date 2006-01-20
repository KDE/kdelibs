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

#include <QTextCodec>

#include <kcodecs.h>
#include <kdebug.h>

#include "vcardparser.h"

#define FOLD_WIDTH 75

using namespace KABC;

static void addEscapes( QByteArray &str )
{
  str.replace( '\\', (char*)"\\\\" );
  str.replace( ',', (char*)"\\," );
  str.replace( '\n', (char*)"\\n" );
}

static void removeEscapes( QByteArray &str )
{
  str.replace( (char*)"\\n", "\n" );
  str.replace( (char*)"\\,", "," );
  str.replace( (char*)"\\\\", "\\" );
}

VCardParser::VCardParser()
{
}

VCardParser::~VCardParser()
{
}

VCard::List VCardParser::parseVCards( const QByteArray& text )
{
  VCard currentVCard;
  VCard::List vCardList;
  QByteArray currentLine;

  const QList<QByteArray> lines = text.split( '\n' );

  bool inVCard = false;
  QList<QByteArray>::ConstIterator it( lines.begin() );
  QList<QByteArray>::ConstIterator linesEnd( lines.end() );
  for ( ; it != linesEnd; ++it ) {
    if ( (*it).trimmed().isEmpty() ) // empty line
      continue;

    if ( (*it).startsWith( ' ' ) || (*it).startsWith( '\t' ) ) { // folded line => append to previous
      currentLine.append( (*it).mid( 1 ) );
      continue;
    } else {
      if ( inVCard && !currentLine.isEmpty() ) { // now parse the line
        int colon = currentLine.indexOf( ':' );
        if ( colon == -1 ) { // invalid line
          currentLine = (*it);
          continue;
        }

        VCardLine vCardLine;
        const QByteArray key = currentLine.left( colon ).trimmed();
        QByteArray value = currentLine.mid( colon + 1 );

        QList<QByteArray> params = key.split( ';' );

        // check for group
        int groupPos = params[ 0 ].indexOf( '.' );
        if ( groupPos != -1 ) {
          vCardLine.setGroup( QString::fromLatin1( params[ 0 ].left( groupPos ) ) );
          vCardLine.setIdentifier( QString::fromLatin1( params[ 0 ].mid( groupPos + 1 ) ) );
        } else
          vCardLine.setIdentifier( QString::fromLatin1( params[ 0 ] ) );

        if ( params.count() > 1 ) { // find all parameters
          QList<QByteArray>::ConstIterator paramIt( params.begin() );
          for ( ++paramIt; paramIt != params.end(); ++paramIt ) {
            QList<QByteArray> pair = (*paramIt).split( '=' );
            if ( pair.count() == 1 ) {
              // correct the fucking 2.1 'standard'
              if ( pair[ 0 ].toLower() == "quoted-printable" ) {
                pair[ 0 ] = "encoding";
                pair.append( "quoted-printable" );
              } else if ( pair[ 0 ].toLower() == "base64" ) {
                pair[ 0 ] = "encoding";
                pair.append( "base64" );
              } else {
                pair.prepend( "type" );
              }
            }
            if ( pair[ 1 ].indexOf( ',' ) != -1 ) { // parameter in type=x,y,z format
              const QList<QByteArray> args = pair[ 1 ].split( ',' );
              QList<QByteArray>::ConstIterator argIt;
              for ( argIt = args.begin(); argIt != args.end(); ++argIt )
                vCardLine.addParameter( QString::fromLatin1( pair[ 0 ].toLower() ), *argIt );
            } else
              vCardLine.addParameter( QString::fromLatin1( pair[ 0 ].toLower() ), pair[ 1 ] );
          }
        }

        removeEscapes( value );

        QByteArray output;

        if ( vCardLine.parameterList().contains( "encoding" ) ) { // have to decode the data
          if ( vCardLine.parameter( "encoding" ).toLower() == "b" ||
               vCardLine.parameter( "encoding" ).toLower() == "base64" )
            KCodecs::base64Decode( value, output );
          else if ( vCardLine.parameter( "encoding" ).toLower() == "quoted-printable" ) {
            // join any qp-folded lines
            while ( value.endsWith( '=' ) && it != linesEnd ) {
              value.chop( 1 ); // remove the '='
              value.append( *it );
              ++it;
            }
            KCodecs::quotedPrintableDecode( value, output );
          } else {
            qDebug( "Unknown vcard encoding type!" );
          }
        } else
          output = value;

        if ( vCardLine.parameterList().contains( "charset" ) ) { // have to convert the data
          QTextCodec *codec = QTextCodec::codecForName( vCardLine.parameter( "charset" ).toLatin1() );
          if ( codec ) {
            vCardLine.setValue( codec->toUnicode( output ) );
          } else {
            vCardLine.setValue( QString::fromUtf8( output ) );
          }
        } else
          vCardLine.setValue( QString::fromUtf8( output ) );

        currentVCard.addLine( vCardLine );
      }

      // we do not save the start and end tag as vcardline
      if ( (*it).toLower().startsWith( "begin:vcard" ) ) {
        inVCard = true;
        currentLine.clear();
        currentVCard.clear(); // flush vcard
        continue;
      }

      if ( (*it).toLower().startsWith( "end:vcard" ) ) {
        inVCard = false;
        vCardList.append( currentVCard );
        currentLine.clear();
        currentVCard.clear(); // flush vcard
        continue;
      }

      currentLine = (*it);
    }
  }

  return vCardList;
}

QByteArray VCardParser::createVCards( const VCard::List& list )
{
  QByteArray text;
  QByteArray textLine;
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
        QVariant val = (*lineIt).value();
        if ( val.isValid() && !val.toString().isEmpty() ) {
          if ( (*lineIt).hasGroup() )
            textLine = (*lineIt).group().toLatin1() + "." + (*lineIt).identifier().toLatin1();
          else
            textLine = (*lineIt).identifier().toLatin1();

          params = (*lineIt).parameterList();
          hasEncoding = false;
          if ( params.count() > 0 ) { // we have parameters
            for ( paramIt = params.begin(); paramIt != params.end(); ++paramIt ) {
              if ( (*paramIt) == "encoding" ) {
                hasEncoding = true;
                encodingType = (*lineIt).parameter( "encoding" ).toLower();
              }

              values = (*lineIt).parameters( *paramIt );
              for ( valueIt = values.constBegin(); valueIt != values.constEnd(); ++valueIt ) {
                textLine.append( ";" + (*paramIt).toLatin1().toUpper() );
                if ( !(*valueIt).isEmpty() )
                  textLine.append( "=" + (*valueIt).toLatin1() );
              }
            }
          }



          QString value = (*lineIt).value().toString();
          QByteArray input, output;

          // handle charset
          if ( (*lineIt).parameterList().contains( "charset" ) ) { // have to convert the data
            QTextCodec *codec = QTextCodec::codecForName( (*lineIt).parameter( "charset" ).toLatin1() );
            if ( codec ) {
              input = codec->fromUnicode( value );
            } else {
              input = value.toUtf8();
            }
          } else
            input = value.toUtf8();

          // handle encoding
          if ( hasEncoding ) { // have to encode the data
            if ( encodingType == "b" ) {
              KCodecs::base64Encode( input, output );
            } else if ( encodingType == "quoted-printable" ) {
              KCodecs::quotedPrintableEncode( input, output, false );
            }
          } else
            output = input;

          addEscapes( output );
          textLine.append( ":" + output );

          if ( textLine.length() > FOLD_WIDTH ) { // we have to fold the line
            for ( int i = 0; i <= ( textLine.length() / FOLD_WIDTH ); ++i )
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
