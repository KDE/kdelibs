/*
    Copyright (c) 2001 Marc Mutz <mutz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2.0
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301 USA
*/

#include "kstringvalidator.h"

#include <kdebug.h>

class KStringListValidator::Private
{
  public:
    QStringList mStringList;
    bool mRejecting : 1;
    bool mFixupEnabled : 1;
};

//
// KStringListValidator
//
KStringListValidator::KStringListValidator( const QStringList &list, bool rejecting,
                                            bool fixupEnabled, QObject *parent )
  : QValidator( parent ),
    d( new Private )
{
  d->mStringList = list;
  d->mRejecting = rejecting;
  d->mFixupEnabled = fixupEnabled;
}

KStringListValidator::~KStringListValidator()
{
  delete d;
}

QValidator::State KStringListValidator::validate( QString & input, int& ) const
{
  if ( input.isEmpty() )
    return Intermediate;

  if ( isRejecting() ) // anything not in mStringList is acceptable:
    if ( !d->mStringList.contains( input ) )
      return Acceptable;
    else
      return Intermediate;
  else // only what is in mStringList is acceptable:
    if ( d->mStringList.contains( input ) )
      return Acceptable;
    else
      for ( QStringList::ConstIterator it = d->mStringList.constBegin(); it != d->mStringList.constEnd() ; ++it )
        if ( (*it).startsWith( input ) || input.startsWith( *it ) )
          return Intermediate;

  return Invalid;
}

void KStringListValidator::fixup( QString& ) const
{
  if ( !isFixupEnabled() )
    return;

  // warn (but only once!) about non-implemented fixup():
  static bool warn = true;
  if ( warn ) {
    kDebug() << "KStringListValidator::fixup() isn't yet implemented!";
    warn = false;
  }
}

void KStringListValidator::setRejecting( bool rejecting )
{
  d->mRejecting = rejecting;
}

bool KStringListValidator::isRejecting() const
{
  return d->mRejecting;
}

void KStringListValidator::setFixupEnabled( bool fixupEnabled )
{
  d->mFixupEnabled = fixupEnabled;
}

bool KStringListValidator::isFixupEnabled() const
{
  return d->mFixupEnabled;
}

void KStringListValidator::setStringList( const QStringList &list )
{
  d->mStringList = list;
}

QStringList KStringListValidator::stringList() const
{
  return d->mStringList;
}

//
// KMimeTypeValidator
//

#define ALLOWED_CHARS "!#-'*+.0-9^-~+-"

class KMimeTypeValidator::Private
{
};

KMimeTypeValidator::KMimeTypeValidator( QObject* parent )
 : QValidator( parent ),
   d( 0 )
{
}

KMimeTypeValidator::~KMimeTypeValidator()
{
  delete d;
}

QValidator::State KMimeTypeValidator::validate( QString &input, int& ) const
{
  if ( input.isEmpty() )
    return Intermediate;

  QRegExp acceptable( "[" ALLOWED_CHARS "]+/[" ALLOWED_CHARS "]+", Qt::CaseInsensitive );
  if ( acceptable.exactMatch( input ) )
    return Acceptable;

  QRegExp intermediate( "[" ALLOWED_CHARS "]*/?[" ALLOWED_CHARS "]*", Qt::CaseInsensitive );
  if ( intermediate.exactMatch( input ) )
    return Intermediate;

  return Invalid;
}

void KMimeTypeValidator::fixup( QString &input ) const
{
  QRegExp invalidChars("[^/" ALLOWED_CHARS "]+");
  input.remove( invalidChars );
}

#include "kstringvalidator.moc"
