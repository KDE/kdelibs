/* This file is part of the KDE libraries
    Copyright (C) 1999 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KCOMPLETION_H
#define KCOMPLETION_H

#include <qlist.h>
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kglobal.h>

#include "kcompletion_private.h"

/**
 * This class let's you easily use "auto-completion" or "manual-completion"
 * on QString objects. A common use is completing filenames or URLs
 * (see @ref KURLCompletion).
 * But URL-completion is not all, everything should be completable. The user
 * should be able to complete email-addresses, telephone-numbers, commands,
 * SQL queries, ...
 * Everytime your program knows what the user can type into an edit-field, you
 * should offer completion. With KCompletion, this is very easy, and if you are
 * using a LineEdit-widget (@ref KLineEdit), it is even more easy.
 * Basically, you tell a KCompletion-object what strings should be completable
 * and then, whenever the user types something, you call @ref makeCompletion.
 * KLineEdit and (an editable) KComboBox even do this automatically for you.
 *
 * KCompletion offers the completed string via the signal @ref match and
 * all matching strings via the method @ref allMatches.
 *
 * Notice: auto-completion and manual completion work a little different:
 *         auto-completion always returns a complete item as match, while
 *         manual completion may return a partial item, when two or more items
 *         would match. The longest possible match will be returned then.
 *
 *         You don't have to worry much about that though, KCompletion handles
 *         that for you, according to the setting @ref setCompletionMode.
 *         The default setting is globally configured by the user and read
 *         from @ref KGlobal::completionMode.
 *
 * A short example:
 * <pre>
 * KCompletion completion;
   completion.setSorted( true );
 * completion.addItem( "pfeiffer@kde.org" );
 * completion.addItem( "coolo@kde.org" );
 * completion.addItem( "carpdjih@sp.zrz.tu-berlin.de" );
 * completion.addItem( "carp@cs.tu-berlin.de" );
 *
 * debug( completion.makeCompletion( "ca" ).local8Bit() );
 * // In manual-completion-mode, this will be "carp"; in auto-completion-
 * // mode it will return "carp@cs.tu-berlin.de", as that is alphabetically
 * // smaller.
 * // If setSorted was set to false (default), "carpdjih@sp.zrz.tu-berlin.de"
 * // would be completed in auto-completion-mode, as that was inserted before
 * // "carp@cs.tu-berlin.de".
 * </pre>
 *
 * You may inherit from KCompletion and override @ref makeCompletion in special
 * cases (like reading directories/urls and then supplying the contents to
 * KCompletion, as KURLCompletion does), but generally, this is not necessary.
 *
 * @short A generic class for completing QStrings
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @version $Id$
 */
class KCompletion : public QObject
{
  Q_OBJECT

public:
  /**
   * Constructor, nothing special here :)
   */
  KCompletion();

  /**
   * Destructor, nothing special here, either.
   */
  ~KCompletion();

  /**
   * Sets the completion mode to Auto, End of Line or None.
   * If you don't set the mode explicitly, the global default value
   * KGlobal::completionMode() is used.
   * @see completionMode
   * @see KGlobal::completionMode
   */
  void setCompletionMode( KGlobal::Completion mode ) {myCompletionMode = mode;}

  /**
   * Returns the current completion mode.
   * May be different from @ref KGlobal::completionMode, if you explicitly
   * called @ref setCompletionMode.
   * @see setCompletionMode
   */
  KGlobal::Completion completionMode() const { return myCompletionMode; }

  /**
   * Setting this to true makes us go into sorted mode (doh).
   * Completion will then always return the alphabetically first match.
   * If set to false, the order is the same as the items were inserted.
   * Note: this only affects new inserted items, already existing items will
   * stay in the current order. So probably want to call setSorted( true )
   * before inserting items, when you want everything sorted.
   * Default is false, not sorted.
   * @see isSorted
   */
  void setSorted( bool enable ) { mySorting = enable; }

  /**
   * Returns true if the completion-items are alphabetically sorted and false
   * if the order of insertion is used.
   * @see setSorted
   */
  bool isSorted() const { return mySorting; }

  /**
   * @returns a copy of the current list of all matching items
   */
  QStringList allMatches() const { return myLastMatches; }

  /**
   * Enables/disables beeping when
   * @li @ref makeCompletion can't find a match
   * @li there is a partial completion
   * Beeps only in manual-completion mode
   * Default is enabled (does beep).
   * @see isBeepEnabled
   */
  void setEnableBeep( bool enable ) { myBeep = enable; }

  /**
   * Tells you whether KCompletion will issue beeps (@ref KApplication::beep)
   * Beeps only in manual-completion mode
   * Default is enabled (does beep).
   * @see setEnableBeep
   */
  bool isBeepEnabled() const { return myBeep; }


public slots:
  /**
   * Attempts to find an item in the list of available completions,
   * that begins with string. Will either return the first (if more than one
   * match) matching item or QString::null, if no match was found. In the
   * latter case, a beep will be issued, depending on @ref isBeepEnabled.
   * If a match was found, it will also be emitted via the signal @ref match.
   *
   * @returns 	the matching item, or QString::null if there is no matching
   * item.
   */
  virtual QString makeCompletion( const QString& string );

  /**
   * @eturns the next item from the matching-items-list
   * When reaching the beginning, the list is rotated, so it will return
   * the last match. When there is no match, QString::null is returned and
   * a beep will be issued, depending on @ref isBeepEnabled.
   */
  QString previousMatch();

  /**
   * @returns the previous item from the matching-items-list
   * When reaching the last item, the list is rotated, so it will return
   * the first match. When there is no match, QString::null is returned and
   * a beep will be issued, depending on @ref isBeepEnabled.
   */
  QString nextMatch();

  /**
   * Sets the list of items available for completion. Removes all previous
   * items.
   */
  void setItemList( const QStringList& );

  /**
   * Adds an item to the list of available completions.
   */
  void addItem( const QString& );

  /**
   * Removes an item from the list of available completions.
   */
  void removeItem( const QString& );

  /**
   * Clears the list of available completions.
   */
  void clear();


signals:
  /**
   * The matching item. Will be emitted by @ref makeCompletion,
   * @ref previousMatch or @ref nextMatch. May be QString::null if there is
   * no matching item.
   */
  void match( const QString& );


private:
  void 			addItemInternal( const QString& );
  QString 		findCompletion( KCompFork * );
  const QStringList& 	findAllCompletions( const QString& );
  void 			extractStringsFromNode( const QString&,
						const KCompTreeNode * );
  void 			doBeep();

  QStringList 		myLastMatches;
  QStringList 		myMatches;  // FIXME, redundant
  QStringList::Iterator myIterator; // FIXME, redundant
  KGlobal::Completion 	myCompletionMode;

  QString 		myLastString;
  QString 		myLastCompletion;
  KCompTreeNode *	myTreeRoot;
  bool 			mySorting;
  bool 			myBeep;
  KCompForkList 	myForkList;
};




#endif // KCOMPLETION_H
