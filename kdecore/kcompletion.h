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

#include <kcompletion_private.h>

struct KCompletionPrivate;

/**
 * This class let's you easily use "auto-completion", "manual-completion" or
 * "shell completion" on QString objects. A common use is completing filenames
 * or URLs (see @ref KURLCompletion).
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
 * Notice: auto-completion, shell completion and manual completion work
 *         slightly differently:
 *
 * @li auto-completion always returns a complete item as match.
 *     When more than one matching items are available, it will deliver just
 *     the first (depending on sorting order) item. Iterating thru them is
 *     possible via @ref nextMatch and @ref previousMatch.
 *
 * @li manual completion works the same way as auto-completion, the subtle
 *     difference is, that it isn't invoked automatically while the user
 *     is typing, but only when the user presses a special key. The difference
 *     of manual and auto-completion is therefore only visible in UI classes,
 *     KCompletion needs to know about whether to deliver partial matches
 *     (shell completion) or whole matches (auto/manual completion), therefore
 *     @ref KGLobal::CompletionManual and @ref KGlobal::CompletionAuto have the
 *     exact same effect in KCompletion.
 *
 * @li shell completion works like how shells complete filenames.
 *     When multiple matches are available, the longest possible of all matches
 *     is returned (i.e. only a partial item).
 *     Iterating thru all matching items (complete, not partial) is possible
 *     via @ref nextMatch and @ref previousMatch.
 *
 * You don't have to worry much about that though, KCompletion handles
 * that for you, according to the setting @ref setCompletionMode.
 * The default setting is globally configured by the user and read
 * from @ref KGlobal::completionMode.
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
 * You can dynamically update the completable items by removing and adding them
 * whenever you want.
 * For advanced usage, you could even use multiple KCompletion objects (e.g.
 * imagine an editor like kwrite with multiple open files. You could store
 * items of every file in a different KCompletion-object, so that you know (and
 * tell the user) where a completion comes from.
 *
 * Note: KCompletion does not work with strings that contain 0x0 characters
 *       (unicode nul), as this is used internally as a delimiter.
 *
 * You may inherit from KCompletion and override @ref makeCompletion in special
 * cases (like reading directories/urls and then supplying the contents to
 * KCompletion, as KURLCompletion does), but generally, this is not necessary.
 *
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

  // FIXME: copy constructor, assignment constructor...

  /**
   * Destructor, nothing special here, either.
   */
  virtual ~KCompletion();

  /**
   * Attempts to find an item in the list of available completions,
   * that begins with string. Will either return the first (if more than one
   * match) matching item or QString::null, if no match was found. In the
   * latter case, a beep will be issued, depending on @ref isBeepEnabled.
   * If a match was found, it will also be emitted via the signal @ref match.
   *
   * If this is called twice or more often with the same string while no items
   * were added or removed in the meantime, all available completions will be
   * emitted via the signal @ref matches.
   * This happens only in shell-completion-mode.
   *
   * @returns 	the matching item, or QString::null if there is no matching
   * item.
   * @see slotMakeCompletion
   */
  virtual QString makeCompletion( const QString& string );

  /**
   * @eturns the next item from the matching-items-list
   * When reaching the beginning, the list is rotated, so it will return
   * the last match. When there is no match, QString::null is returned and
   * a beep will be issued, depending on @ref isBeepEnabled.
   * @see slotPreviousMatch
   */
  QString previousMatch();

  /**
   * @returns the previous item from the matching-items-list
   * When reaching the last item, the list is rotated, so it will return
   * the first match. When there is no match, QString::null is returned and
   * a beep will be issued, depending on @ref isBeepEnabled.
   * @see slotNextMatch
   */
  QString nextMatch();

  /**
   * @returns the last match. Might be useful if you need to check whether
   * a completion is different from the last one.
   * QString::null is returned when there is no last match.
   */
  const QString& lastMatch() const { return myLastMatch; } // FIXME

  /**
   * Sets the completion mode to Auto/Manual (@see KCompletion documentation),
   * Shell or None.
   * If you don't set the mode explicitly, the global default value
   * KGlobal::completionMode() is used. @ref KGlobal::CompletionNone disables
   * completion.
   * @see completionMode
   * @see KGlobal::completionMode
   */
  void setCompletionMode( KGlobal::Completion mode );

  /**
   * @eturns the current completion mode.
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
   * @eturns true if the completion-items are alphabetically sorted and false
   * if the order of insertion is used.
   * @see setSorted
   */
  bool isSorted() const { return mySorting; }

  /**
   * @returns a list of all matching items. Might take some time, when you
   * have LOTS of items.
   */
  QStringList allMatches() { return findAllCompletions( myLastString ); }

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
   * Attempts to complete "string" and emits the completion via @ref match.
   * Same as @ref makeCompletion (just as a slot).
   * @see makeCompletion
   */
  void slotMakeCompletion( const QString& string ) {
      (void) makeCompletion( string );
  }

  /**
   * Searches the previous matching item and emits it via @ref match
   * Same as @ref previousMatch (just as a slot).
   * @see previousMatch
   */
  void slotPreviousMatch() {
      (void) previousMatch();
  }

  /**
   * Searches the next matching item and emits it via @ref match
   * Same as @ref nextMatch (just as a slot).
   * @see nextMatch
   */
  void slotNextMatch() {
      (void) nextMatch();
  }

  /**
   * @returns true when more than one match is found
   * @see multipleMatches
   */
  bool hasMultipleMatches() const { return !myForkList.isEmpty(); }

  /**
   * Sets the list of items available for completion. Removes all previous
   * items.
   */
  void setItemList( const QStringList& );

  /**
   * Adds an item to the list of available completions.
   * Resets the current item-state (@ref previousMatch and @ref nextMatch won't
   * work anymore).
   */
  void addItem( const QString& );

  /**
   * Removes an item from the list of available completions.
   * Resets the current item-state (@ref previousMatch and @ref nextMatch won't
   * work anymore).
   */
  void removeItem( const QString& );

  /**
   * Clears the list of inserted items.
   */
  void clear();


signals:
  /**
   * The matching item. Will be emitted by @ref makeCompletion,
   * @ref previousMatch or @ref nextMatch. May be QString::null if there is
   * no matching item.
   */
  void match( const QString& );

  /**
   * All matching items. Will be emitted by @ref makeCompletion in shell-
   * completion-mode, when the same string is passed to makeCompletion twice
   * or more often.
   */
  void matches( const QStringList& );

  /**
   * This signal is emitted, when calling @ref makeCompletion and more than
   * one matching item is found.
   * @see hasMultipleMatches
   */
  void multipleMatches();

private:
  void 			addItemInternal( const QString& );
  QString 		findCompletion( const QString& string );
  QString 		findCompletion( KCompFork * );
  const QStringList& 	findAllCompletions( const QString& );
  void 			extractStringsFromNode( const KCompTreeNode *,
						const QString& beginning );
  void 			doBeep();

  QStringList           myMatches;
  KGlobal::Completion   myCompletionMode;

  QString               myLastString;
  QString 		myLastMatch;
  KCompTreeNode *       myTreeRoot;
  KCompForkList         myForkList;
  bool                  mySorting;
  bool                  myBeep;
  bool 			myBackwards;
  int 			myItemIndex; // FIXME
};


#endif // KCOMPLETION_H
