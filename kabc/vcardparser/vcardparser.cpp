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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qregexp.h>

#include <kmdcodec.h>

#include "vcardparser.h"

#define FOLD_WIDTH 75

VCardParser::VCardParser()
{
}

VCardParser::~VCardParser()
{
}

VCard::List VCardParser::parseVCards( const QString& text )
{
  VCard currentVCard;
  VCard::List vCardList;
  QString currentLine;

  QStringList lines = QStringList::split( QRegExp( "[\x0d\x0a]" ), text );
  QStringList::Iterator it;

  bool inVCard = false;
  for ( it = lines.begin(); it != lines.end(); ++it ) {

    if ( (*it).isEmpty() ) // empty line
      continue;

    if ( (*it)[ 0 ] == ' ' || (*it)[ 0 ] == '\t' ) { // folded line => append to previous
      currentLine += (*it).remove( 0, 1 );
      continue;
    } else {
      if ( inVCard && !currentLine.isEmpty() ) { // now parse the line
        int colon = currentLine.find( ':' );
        if ( colon == -1 ) { // invalid line
          currentLine = (*it);
          continue;
        }

        VCardLine vCardLine;
        QString key = currentLine.left( colon ).stripWhiteSpace();
        QString value = currentLine.mid( colon + 1 );

        QStringList params = QStringList::split( ';', key );
        vCardLine.setIdentifier( params[0].lower() );
        if ( params.count() > 1 ) { // find all parameters
          for ( uint i = 1; i < params.count(); ++i ) {
            QStringList pair = QStringList::split( '=', params[i] );
            if ( pair[1].contains( ',' ) ) { // parameter in type=x,y,z format
              QStringList args = QStringList::split( ',', pair[ 1 ] );
              for ( uint j = 0; j < args.count(); ++j )
                vCardLine.addParameter( pair[0].lower(), args[j] );
            } else
              vCardLine.addParameter( pair[0].lower(), pair[1] );
          }
        }

        params = vCardLine.parameterList();
        if ( params.contains( "encoding" ) ) { // have to decode the data
          QByteArray input, output;
          input.setRawData( vCardLine.value().asByteArray(),
                            vCardLine.value().asByteArray().count() );
          if ( vCardLine.parameter( "encoding" ).lower() == "b" )
            KCodecs::base64Decode( input, output );
          else if ( vCardLine.parameter( "encoding" ).lower() == "quoted-printable" )
            KCodecs::quotedPrintableDecode( input, output );
          vCardLine.setValue( output );
        } else
          vCardLine.setValue( value.replace( "\\n", "\n" ) );

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

  VCard::List::ConstIterator cardIt;

  // iterate over the cards
  for ( cardIt = list.begin(); cardIt != list.end(); ++cardIt ) {
    // find version
//    int version = (*cardIt).version();

    text.append( "BEGIN:VCARD\r\n" );

    QStringList idents = (*cardIt).identifiers();
    QStringList::ConstIterator identIt;
    for ( identIt = idents.begin(); identIt != idents.end(); ++identIt ) {
      VCard card = (*cardIt);
      VCardLine::List lines = card.lines( (*identIt) );
      VCardLine::List::Iterator lineIt;

      // iterate over the lines
      for ( lineIt = lines.begin(); lineIt != lines.end(); ++lineIt ) {
        if ( !(*lineIt).value().asString().isEmpty() ) {
          QString textLine;
          
          textLine.append( (*lineIt).identifier().upper() );

          QStringList params = (*lineIt).parameterList();
          if ( params.count() > 0 ) { // we have parameters
            QStringList::Iterator paramIt;
            for ( paramIt = params.begin(); paramIt != params.end(); ++paramIt ) {
              QStringList values = (*lineIt).parameters( *paramIt );
              QStringList::Iterator valueIt;
              for ( valueIt = values.begin(); valueIt != values.end(); ++valueIt ) {
                textLine.append( ";" + (*paramIt).upper() );
                if ( !(*valueIt).isEmpty() )
                  textLine.append( "=" + (*valueIt) );
              }
            }
          }

          params = (*lineIt).parameterList();
          if ( params.contains( "encoding" ) ) { // have to encode the data
            QByteArray input, output;
            input.setRawData( (*lineIt).value().asByteArray(),
                              (*lineIt).value().asByteArray().count() );
            if ( (*lineIt).parameter( "encoding" ).lower() == "b" )
              KCodecs::base64Decode( input, output );
            else if ( (*lineIt).parameter( "encoding" ).lower() == "quoted-printable" )
              KCodecs::quotedPrintableDecode( input, output );
            textLine.append( ":" + QString( output ) );
          } else
            textLine.append( ":" + (*lineIt).value().asString().replace( "\n", "\\n" ) );

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
