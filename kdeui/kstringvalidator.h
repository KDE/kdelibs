/*
    kstringvalidator.h

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

#ifndef KSTRINGVALIDATOR_H
#define KSTRINGVALIDATOR_H

#include <qvalidator.h>
#include <qstringlist.h>

#include <kdelibs_export.h>

/**
 * @short A QValidator to (dis)allow certain strings
 *
 * This validator allows you to accept only certain or to accept all
 * but certain strings.
 *
 * When used in rejecting mode, accepts only strings not in the
 * stringlist. This mode is the default and comes in handy when asking
 * the user for a name of some listed entity. Set the list of already
 * used names to prevent the user from entering duplicate names.
 *
 * When used in non-rejecting mode, accepts only strings that appear
 * in the stringlist. Use with care! From a user's point of view this
 * mode is hard to grasp.
 *
 * This validator can also fix strings. In rejecting mode, a number
 * will be appended to the string until it is Acceptable. E.g. if
 * "foo" and "foo 1" are in the stringlist, then fixup will change
 * "foo" to "foo 2", provided "foo 2" isn't in the list of forbidden
 * strings.
 *
 * In accepting mode, when the input starts with an Acceptable
 * substring, truncates to the longest Acceptable string. When the
 * input is the start of an Acceptable string, completes to the
 * shortest Acceptable string.
 *
 * NOTE: fixup isn't yet implemented.
 *
 * @author Marc Mutz <mutz@kde.org>
 **/
class KDEUI_EXPORT KStringListValidator : public QValidator {
  Q_OBJECT
  Q_PROPERTY( QStringList stringList READ stringList WRITE setStringList )
  Q_PROPERTY( bool rejecting READ isRejecting WRITE setRejecting )
  Q_PROPERTY( bool fixupEnabled READ isFixupEnabled WRITE setFixupEnabled )
public:
  /** Construct a new validator.
   *
   * @param list         The list of strings to (dis)allow.
   * @param rejecting    Selects the validator's mode
   *                     (rejecting: true; accepting: false)
   * @param fixupEnabled Selects whether to fix strings or not.
   * @param parent Passed to lower level constructor.
   * @param name Passed to lower level constructor
   *
   **/
  KStringListValidator( const QStringList & list=QStringList(),
			bool rejecting=true, bool fixupEnabled=false,
			QObject * parent=0, const char * name=0 )
    : QValidator( parent ), mStringList( list ),
      mRejecting( rejecting ), mFixupEnabled( fixupEnabled ) { setObjectName(name);}

  virtual State validate( QString & input, int & pos ) const;
  virtual void fixup( QString & input ) const;

  void setRejecting( bool rejecting ) { mRejecting = rejecting; }
  bool isRejecting() const { return mRejecting; }

  void setFixupEnabled( bool fixupEnabled ) { mFixupEnabled = fixupEnabled; }
  bool isFixupEnabled() const { return mFixupEnabled; }

  void setStringList( const QStringList & list ) { mStringList = list; }
  QStringList stringList() const { return mStringList; }

protected:
  QStringList mStringList;
  bool        mRejecting;
  bool        mFixupEnabled;
private:
  class KStringListValidator* d;
};

/**
 * @short A QValidator for mime types.
 *
 * This validator allows you to validate mimetype names
 * (e.g. text/plain, image/jpeg). Note that the validation is only
 * syntactically. It will e.g. not reject "foo/bar", although that
 * particular mime type isn't yet registered. It suffices for the name
 * to adhere to the production
 *
 * \code
 * mime-type := token "/" token ; 'token' is defined in rfc2045
 * \endcode
 *
 * The fixup will simply remove all non-token characters.
 *
 * @author Marc Mutz <mutz@kde.org>
 **/
class KDEUI_EXPORT KMimeTypeValidator : public QValidator
{
  Q_OBJECT
public:
  KMimeTypeValidator( QObject* parent, const char* name=0)
    : QValidator( parent ) { setObjectName(name);}

  /**
   * Checks for well-formed mimetype. Returns
   * @li Acceptable iff input ~= /^[:allowed chars:]+\/[:allowed chars:]+$/
   * @li Intermediate iff input ~= /^[:allowed chars:]*\/?[:allowed chars:]*$/
   * @li Invalid else
   */
  virtual State validate( QString & input, int & pos ) const;
  /**
   * Removes all characters that are forbidden in mimetypes.
   */
  virtual void fixup( QString & input ) const;
private:
  class KMimeTypeValidator* d;
};

#endif // KSTRINGVALIDATOR_H
