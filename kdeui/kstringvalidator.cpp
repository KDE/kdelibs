/*
    kstringvalidator.cpp

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA
*/

#include "kstringvalidator.h"
#include "kdebug.h"

//
// KStringListValidator
//

QValidator::State KStringListValidator::validate( QString & input, int& ) const {
  if ( input.isEmpty() ) return Intermediate;

  if ( isRejecting() ) // anything not in mStringList is acceptable:
    if ( mStringList.find( input ) == mStringList.end() )
      return Acceptable;
    else
      return Intermediate;
  else // only what is in mStringList is acceptable:
    if ( mStringList.find( input ) != mStringList.end() )
      return Acceptable;
    else
      for ( QStringList::ConstIterator it = mStringList.begin() ;
	    it != mStringList.end() ; ++it )
	if ( (*it).startsWith( input ) || input.startsWith( *it ) )
	  return Intermediate;

  return Invalid;
}

void KStringListValidator::fixup( QString & /* input */ ) const {
  if ( !isFixupEnabled() ) return;
  // warn (but only once!) about non-implemented fixup():
  static bool warn = true;
  if ( warn ) {
    kdDebug() << "KStringListValidator::fixup() isn't yet implemented!"
	      << endl;
    warn = false;
  }
}

//
// KMimeTypeValidator
//

#define ALLOWED_CHARS "!#-'*+.0-9^-~+-"

QValidator::State KMimeTypeValidator::validate( QString & input, int& ) const
{
  if ( input.isEmpty() )
    return Intermediate;

  QRegExp acceptable( "[" ALLOWED_CHARS "]+/[" ALLOWED_CHARS "]+",
		      false /*case-insens.*/);
  if ( acceptable.exactMatch( input ) )
    return Acceptable;

  QRegExp intermediate( "[" ALLOWED_CHARS "]*/?[" ALLOWED_CHARS "]*",
			false /*case-insensitive*/);
  if ( intermediate.exactMatch( input ) )
    return Intermediate;

  return Invalid;
}

void KMimeTypeValidator::fixup( QString & input ) const
{
  QRegExp invalidChars("[^/" ALLOWED_CHARS "]+");
  input.replace( invalidChars, "");
}

#include "kstringvalidator.moc"
