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

#include <qcstring.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <qvaluelist.h>

#include <kmdcodec.h>
#include <kdebug.h>

#include "vcardparser.h"

#define FOLD_WIDTH 75

using namespace KABC;

typedef QValueList<QCString> QCStringList;

QValueList<QCString> splitCString( const QCString &str, char sep )
{
    QValueList<QCString> list;
    int start = 0;
    int end;
    while ((end = str.find(sep, start)) != -1) {
        list.append(str.mid(start, end - start));
        start = end + 1;
    }
    list.append(str.mid(start));

    return list;
}

QValueList<QCString> splitCString( const QCString &str, const QRegExp &exp )
{
    QValueList<QCString> list;
    int start = 0;
    int end;
    while ((end = str.find(exp, start)) != -1) {
        list.append(str.mid(start, end - start));
        start = end + 1;
    }
    list.append(str.mid(start));

    return list;
}

bool cStringStartsWith( const QCString &str, const QCString &pattern )
{
  const int length = pattern.length();
  if ( length == 0 )
    return true;

  const QCString part = str.left( length );
  return (pattern == part);
}

static void addEscapes( QCString &str )
{
  str.replace( '\\', "\\\\" );
  str.replace( ',', "\\," );
  str.replace( '\r', "\\r" );
  str.replace( '\n', "\\n" );
}

static void removeEscapes( QCString &str )
{
  str.replace( "\\r", "\r" );
  str.replace( "\\n", "\n" );
  str.replace( "\\,", "," );
  str.replace( "\\\\", "\\" );
}

VCardParser::VCardParser()
{
}

VCardParser::~VCardParser()
{
}

VCard::List VCardParser::parseVCards( const QString& text )
{
  return parseVCardsRaw( text.utf8() );
}

VCard::List VCardParser::parseVCardsRaw( const QCString& text )
{

  static QRegExp sep( "[\x0d\x0a]" );

  VCard currentVCard;
  VCard::List vCardList;
  QCString currentLine;

  const QCStringList lines = splitCString( text, sep );
  QCStringList::ConstIterator it;

  bool inVCard = false;
  QCStringList::ConstIterator linesEnd( lines.end() );
  for ( it = lines.begin(); it != linesEnd; ++it ) {

    if ( (*it).isEmpty() ) // empty line
      continue;

    if ( (*it)[ 0 ] == ' ' || (*it)[ 0 ] == '\t' ) { // folded line => append to previous
      currentLine.append( (*it).mid( 1 ) );
      continue;
    } else {
      if ( inVCard && !currentLine.isEmpty() ) { // now parse the line
        int colon = currentLine.find( ':' );
        if ( colon == -1 ) { // invalid line
          currentLine = (*it);
          continue;
        }

        VCardLine vCardLine;
        const QCString key = currentLine.left( colon ).stripWhiteSpace();
        QCString value = currentLine.mid( colon + 1 );

        QCStringList params = splitCString( key, ';' );

        // check for group
        if ( params[0].find( '.' ) != -1 ) {
          const QCStringList groupList = splitCString( params[0], '.' );
          vCardLine.setGroup( QString::fromLatin1( groupList[0] ) );
          vCardLine.setIdentifier( QString::fromLatin1( groupList[1] ) );
        } else
          vCardLine.setIdentifier( QString::fromLatin1( params[0] ) );

        if ( params.count() > 1 ) { // find all parameters
          QCStringList::ConstIterator paramIt = params.begin();
          for ( ++paramIt; paramIt != params.end(); ++paramIt ) {
            QCStringList pair = splitCString( *paramIt, '=' );
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
              const QCStringList args = splitCString( pair[ 1 ], ',' );
              QCStringList::ConstIterator argIt;
              for ( argIt = args.begin(); argIt != args.end(); ++argIt )
                vCardLine.addParameter( QString::fromLatin1( pair[0].lower() ), QString::fromLatin1( *argIt ) );
            } else
              vCardLine.addParameter( QString::fromLatin1( pair[0].lower() ), QString::fromLatin1( pair[1] ) );
          }
        }

        removeEscapes( value );

        QByteArray output;
        bool wasBase64Encoded = false;

        if ( vCardLine.parameterList().findIndex( "encoding" ) != -1 ) { // have to decode the data
          QByteArray input = value;
          if ( vCardLine.parameter( "encoding" ).lower() == "b" ||
               vCardLine.parameter( "encoding" ).lower() == "base64" ) {
            KCodecs::base64Decode( input, output );
            wasBase64Encoded = true;
          }
          else if ( vCardLine.parameter( "encoding" ).lower() == "quoted-printable" ) {
            // join any qp-folded lines
            while ( value.at( value.length() - 1 ) == '=' && it != linesEnd ) {
              value = value.remove( value.length() - 1, 1 ) + (*it);
              ++it;
            }
            input = value;
            KCodecs::quotedPrintableDecode( input, output );
          }
        } else {
          output = value;
        }

        if ( vCardLine.parameterList().findIndex( "charset" ) != -1 ) { // have to convert the data
          QTextCodec *codec =
            QTextCodec::codecForName( vCardLine.parameter( "charset" ).latin1() );
          if ( codec ) {
            vCardLine.setValue( codec->toUnicode( output ) );
          } else {
#if defined(KABC_VCARD_ENCODING_FIX)
            vCardLine.setValue( QString::fromUtf8( output ) );
#else
            vCardLine.setValue( QString::fromLatin1( output ) );
#endif
          }
        } else if ( wasBase64Encoded ) {
            vCardLine.setValue( output );
        } else {
#if defined(KABC_VCARD_ENCODING_FIX)
            vCardLine.setValue( QString::fromUtf8( output ) );
#else
            vCardLine.setValue( QString::fromLatin1( output ) );
#endif
        }

        currentVCard.addLine( vCardLine );
      }

      // we do not save the start and end tag as vcardline
      if ( cStringStartsWith( (*it).lower(), QCString( "begin:vcard" ) ) ) {
        inVCard = true;
        currentLine = QCString();
        currentVCard.clear(); // flush vcard
        continue;
      }

      if ( cStringStartsWith( (*it).lower(), QCString( "end:vcard" ) ) ) {
        inVCard = false;
        vCardList.append( currentVCard );
        currentLine = QCString();
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
  return QString::fromUtf8( createVCardsRaw( list ) );
}

QCString VCardParser::createVCardsRaw( const VCard::List& list )
{
  QCString text;
  QCString textLine;
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

//  text.reserve( list.size() * 300 ); // reserve memory to be more efficient

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
            textLine = (*lineIt).group().latin1() + QCString( "." ) + (*lineIt).identifier().latin1();
          else
            textLine = (*lineIt).identifier().latin1();

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
                textLine.append( QCString( ";" ) + (*paramIt).upper().latin1() );
                if ( !(*valueIt).isEmpty() )
                  textLine.append( QCString( "=" ) + (*valueIt).latin1() );
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

            QCString value( output );
            addEscapes( value );
            textLine.append( ":" + value );
          } else {
            QCString value( (*lineIt).value().toString().utf8() );
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
