/*
 Copyright (c) 2003 Trolltech AS
 Copyright (c) 2003 Scott Wheeler <wheeler@kde.org>

 This file is part of the KDE libraries

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

#include <QHash>
#include <QRegExp>
#include <QStringList>
#include <QTimer>
#include <QKeyEvent>
#include <QTextEdit>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include "ksyntaxhighlighter.h"

KSyntaxHighlighter::KSyntaxHighlighter( QTextEdit *textEdit,
                                        bool colorQuoting,
                                        const QColor& depth0,
                                        const QColor& depth1,
                                        const QColor& depth2,
                                        const QColor& depth3,
                                        SyntaxMode mode )
  : QSyntaxHighlighter( textEdit ),
    d( new Private )
{
  d->mEnabled = colorQuoting;
  d->mColor1 = depth0;
  d->mColor2 = depth1;
  d->mColor3 = depth2;
  d->mColor4 = depth3;
  d->mColor5 = depth0;

  d->mMode = mode;
}

KSyntaxHighlighter::~KSyntaxHighlighter()
{
  delete d;
}

void KSyntaxHighlighter::highlightBlock ( const QString & text )
{
  if ( !d->mEnabled ) {
    //reset color.
    //setFormat( 0, text.length(), document()->paletteForegroundColor() );
    setCurrentBlockState( 0 );
    return;
  }

  QString simplified = text;
  simplified = simplified.replace( QRegExp( "\\s" ), QString() ).replace( '|', QLatin1String(">") );
  while ( simplified.startsWith( QLatin1String(">>>>") ) )
    simplified = simplified.mid( 3 );

  if ( simplified.startsWith( QLatin1String(">>>") ) || simplified.startsWith( QString::fromLatin1("> >	>") ) )
    setFormat( 0, text.length(), d->mColor2 );
  else if ( simplified.startsWith( QLatin1String(">>") ) || simplified.startsWith( QString::fromLatin1("> >") ) )
    setFormat( 0, text.length(), d->mColor3 );
  else if ( simplified.startsWith( QLatin1String(">") ) )
    setFormat( 0, text.length(), d->mColor4 );
  else
    setFormat( 0, text.length(), d->mColor5 );

  setCurrentBlockState( 0 );
}

#include "ksyntaxhighlighter.moc"
