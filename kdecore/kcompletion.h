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

class KCompletionPrivate;

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
 * and then, whenever the user types something, you call @ref makeCompletion().
 * KLineEdit and (an editable) KComboBox even do this automatically for you.
 *
 * KCompletion offers the completed string via the signal @ref match() and
 * all matching strings via the method @ref allMatches().
 *
 * Notice: auto-completion, shell completion and manual completion work
 *         slightly differently:
 *
 * @li auto-completion always returns a complete item as match.
 *     When more than one matching items are available, it will deliver just
 *     the first (depending on sorting order) item. Iterating thru them is
 *     possible via @ref nextMatch() and @ref previousMatch().
 *
 * @li manual completion works the same way as auto-completion, the subtle
 *     difference is, that it isn't invoked automatically while the user
 *     is typing, but only when the user presses a special key. The difference
 *     of manual and auto-completion is therefore only visible in UI classes,
 *     KCompletion needs to know about whether to deliver partial matches
 *     (shell completion) or whole matches (auto/manual completion), therefore
 *     @ref KGLobal::CompletionMan and @ref KGlobal::CompletionAuto have the
 *     exact same effect in KCompletion.
 *
 * @li shell completion works like how shells complete filenames.
 *     When multiple matches are available, the longest possible of all matches
 *     is returned (i.e. only a partial item).
 *     Iterating thru all matching items (complete, not partial) is possible
 *     via @ref nextMatch() and @ref previousMatch().
 *
 * You don't have to worry much about that though, KCompletion handles
 * that for you, according to the setting @ref setCompletionMode().
 * The default setting is globally configured by the user and read
 * from @ref KGlobal::completionMode.
 *
 * A short example:
 * <pre>
 * KCompletion completion;
 * completion.setSorted( true );
 * completion.addItem( "pfeiffer@kde.org" );
 * completion.addItem( "coolo@kde.org" );
 * completion.addItem( "carpdjih@sp.zrz.tu-berlin.de" );
 * completion.addItem( "carp@cs.tu-berlin.de" );
 *
 * debug( completion.makeCompletion( "ca" ).local8Bit() );
 * </pre>
 * In shell-completion-mode, this will be "carp"; in auto-completion-
 * mode it will return "carp@cs.tu-berlin.de", as that is alphabetically
 * smaller.
 * If setSorted was set to false (default), "carpdjih@sp.zrz.tu-berlin.de"
 * would be completed in auto-completion-mode, as that was inserted before
 * "carp@cs.tu-berlin.de".
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
 * You may inherit from KCompletion and override @ref makeCompletion() in
 * special cases (like reading directories/urls and then supplying the
 * contents to KCompletion, as KURLCompletion does), but generally, this is
 * not necessary.
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
     * latter case, a beep will be issued, depending on @ref isBeepEnabled().
     * If a match was found, it will also be emitted via the signal @ref match().
     *
     * If this is called twice or more often with the same string while no items
     * were added or removed in the meantime, all available completions will be
     * emitted via the signal @ref matches().
     * This happens only in shell-completion-mode.
     *
     * @returns 	the matching item, or QString::null if there is no matching
     * item.
     * @see #slotMakeCompletion
     */
    virtual QString makeCompletion( const QString& string );

    /**
     * @returns the next item from the matching-items-list
     * When reaching the beginning, the list is rotated, so it will return
     * the last match. When there is no match, QString::null is returned and
     * a beep will be issued, depending on @ref isBeepEnabled().
     * @see #slotPreviousMatch
     */
    QString previousMatch();

    /**
     * @returns the previous item from the matching-items-list
     * When reaching the last item, the list is rotated, so it will return
     * the first match. When there is no match, QString::null is returned and
     * a beep will be issued, depending on @ref isBeepEnabled().
     * @see #slotNextMatch
     */
    QString nextMatch();

    /**
     * @returns the last match. Might be useful if you need to check whether
     * a completion is different from the last one.
     * QString::null is returned when there is no last match.
     */
    const QString& lastMatch() const { return myLastMatch; } // FIXME


    /**
     * @returns a list of all items inserted into KCompletion. This is useful
     * if you need to save the state of a KCompletion object and restore it
     * later.
     * @see #setItems
     */
    QStringList items() const;


    /**
     * Sets the completion mode to Auto/Manual (@ref KCompletion documentation),
     * Shell or None.
     * If you don't set the mode explicitly, the global default value
     * KGlobal::completionMode() is used. @ref KGlobal::CompletionNone disables
     * completion.
     * @see #completionMode
     * @see #KGlobal::completionMode
     */
    void setCompletionMode( KGlobal::Completion mode );

    /**
     * @returns the current completion mode.
     * May be different from @ref KGlobal::completionMode(), if you explicitly
     * called @ref setCompletionMode().
     * @see #setCompletionMode
     */
    KGlobal::Completion completionMode() const { return myCompletionMode; }

    /**
     * Setting this to true makes us go into sorted mode (doh).
     * Completion will then always return the alphabetically first match.
     * If set to false, the order is the same as the items were inserted.
     * Note: this only affects new inserted items, already existing items will
     * stay in the current order. So you probably want to call setSorted( true )
     * before inserting items, when you want everything sorted.
     * Default is false, not sorted.
     * @see #isSorted
     */
    void setSorted( bool enable ) { mySorting = enable; }

    /**
     * Setting this to true makes KCompletion behave case insensitively.
     * E.g. makeCompletion( "CA" ); might return "carp@cs.tu-berlin.de".
     * Default is false (case sensitive).
     * @see #ignoreCase
     */
    //  void setIgnoreCase( bool ignoreCase ) { myIgnoreCase = ignoreCase; }

    /**
     * @returns whether KCompletion acts case insensitively or not.
     * Default is false (case sensitive).
     * @see #setIgnoreCase
     */
    //  bool ignoreCase() const { return myIgnoreCase; }

    /**
     * @returns true if the completion-items are alphabetically sorted and false
     * if the order of insertion is used.
     * @see #setSorted
     */
    bool isSorted() const { return mySorting; }

    /**
     * @returns a list of all matching items. Might take some time, when you
     * have LOTS of items.
     */
    QStringList allMatches() { return findAllCompletions( myLastString ); }

    /**
     * Enables playing a sound when
     * @li @ref makeCompletion() can't find a match
     * @li there is a partial completion
     *
     * Sounds are only played in shell-completion mode. Default is enabled
     * @see #disableSounds
     * @see #isSoundEnabled
     */
    void enableSounds() { myBeep = true; }

    /**
     * Disables playing a sound when
     * @li @ref makeCompletion() can't find a match
     * @li there is a partial completion
     *
     * Sounds are only played in shell-completion mode. Default is enabled
     * @see #enableSounds
     * @see #isSoundEnabled
     */
    void disableSounds() { myBeep = false; }

    /**
     * Tells you whether KCompletion will issue beeps (@ref KApplication::beep())
     * Beeps only in manual-completion mode
     * Default is enabled
     * @see #enableSounds
     * @see #disableSounds
     */
    bool isSoundsEnabled() const { return myBeep; }


public slots:
    /**
     * Attempts to complete "string" and emits the completion via @ref match().
     * Same as @ref makeCompletion() (just as a slot).
     * @see #makeCompletion
     */
    void slotMakeCompletion( const QString& string ) {
	(void) makeCompletion( string );
    }

    /**
     * Searches the previous matching item and emits it via @ref match()
     * Same as @ref previousMatch() (just as a slot).
     * @see #previousMatch
     */
    void slotPreviousMatch() {
	(void) previousMatch();
    }

    /**
     * Searches the next matching item and emits it via @ref match()
     * Same as @ref nextMatch() (just as a slot).
     * @see #nextMatch
     */
    void slotNextMatch() {
	(void) nextMatch();
    }

    /**
     * @returns true when more than one match is found
     * @see #multipleMatches
     */
    bool hasMultipleMatches() const { return myHasMultipleMatches; }

    /**
     * Sets the list of items available for completion. Removes all previous
     * items.
     * @see #items
     */
    void setItems( const QStringList& );

    /**
     * Adds an item to the list of available completions.
     * Resets the current item-state (@ref previousMatch() and @ref nextMatch()
     * won't work anymore).
     */
    void addItem( const QString& );

    /**
     * Removes an item from the list of available completions.
     * Resets the current item-state (@ref previousMatch() and @ref nextMatch()
     * won't work anymore).
     */
    void removeItem( const QString& );

    /**
     * Clears the list of inserted items.
     */
    void clear();


signals:
    /**
     * The matching item. Will be emitted by @ref makeCompletion(),
     * @ref previousMatch() or @ref nextMatch(). May be QString::null if there is
     * no matching item.
     */
    void match( const QString& );

    /**
     * All matching items. Will be emitted by @ref makeCompletion() in shell-
     * completion-mode, when the same string is passed to makeCompletion twice
     * or more often.
     */
    void matches( const QStringList& );

    /**
     * This signal is emitted, when calling @ref makeCompletion() and more than
     * one matching item is found.
     * @see #hasMultipleMatches
     */
    void multipleMatches();

private:
    void 		addItemInternal( const QString& );
    QString 		findCompletion( const QString& string );
    const QStringList& 	findAllCompletions( const QString& );
    void 		extractStringsFromNode( const KCompTreeNode *,
						const QString& beginning,
						QStringList *matches ) const;

    void 		doBeep();

    QStringList         myMatches;
    KGlobal::Completion myCompletionMode;

    QString             myLastString;
    QString 		myLastMatch;
    KCompTreeNode *     myTreeRoot;
    QStringList 	myRotations;
    bool                mySorting;
    bool                myBeep;
    bool 		myBackwards;
    bool 		myIgnoreCase;
    bool 		myHasMultipleMatches;
    int 		myItemIndex; // FIXME
    uint 		myRotationIndex;

    KCompletionPrivate *d;
};


 /**
 * An abstract base class for adding completion feature
 * into widgets.
 *
 * This is a convienence class that tries to provide
 * the common functions needed to add support for
 * completion into widgets.  Refer to @ref KLineEdit
 * or @ref KComboBox to see how to such support can be
 * added using this base class.
 *
 * NOTE: Do not forget to provide an implementation for
 * the protected pure virtual method @ref connectSignals()
 * if you do not want the class to be abstract.
 *
 * @short An abstract class for using KCompletion in widgets
 * @author Dawit Alemayehu <adawit@earthlink.net>
 */
class KCompletionBase
{

public:

    /**
    * Default constructor.
    */
    KCompletionBase();

    /**
    * Basic destructor.
    */
    virtual ~KCompletionBase();

    /**
    * Returns a pointer to the current completion object.
    *
    * If the object does not exisit, it is automatically
    * created.  Note that the completion object created
    * here is used by default to handle the signals
    * internally.  It is also deleted when this object's
    * destructor is invoked.  If you do not want these
    * default settings, use @ref setAutoDeleteCompletionObject
    * and @ref setHandleSignals to change the behavior.
    * Alternatively, you can set the boolean parameter to
    * false to disable the automatic handling of the signals
    * by this object.  Note that the boolean argument will be
    * ignored if there already exists a completion object since
    * no new object needs to be created. You need to use either
    * @ref setHandleSignals or @ref setCompletionObject for
    * such cases depending on your requirement.
    *
    * @param hsig if true, handles signals internally.
    * @return a pointer the completion object.
    */
    KCompletion* completionObject( bool hsig = true );

    /**
    * Sets up the completion object to be used.
    *
    * This method assigns the completion object and sets it
    * up to automatically handle the completion and rotation
    * signals internally.  You should use this function if
    * you want to share one completion object among you widgets
    * or need to use a customized completion object.
    *
    * The object assigned through this method is not deleted
    * when this object's destructor is invoked unless you
    * explicitly call @ref setAutoDeleteCompletionObject after
    * calling this method.  Also if you do not want the signals
    * to be handled by an internal implementation, be sure to
    * set the bool argument to false.
    *
    * @param compObj a @ref KCompletion or a derived child object.
    * @param hsig if true, handles signals internally.
    */
    virtual void setCompletionObject( KCompletion* compObj, bool hsig = true );

    /**
    * Enables this object to handle completion signals
    * internally.
    *
    * This function simply assigns the boolean value that
    * indicates whether it should handle signals or not.
    * All child objects must provide an implementation for
    * @ref connectSignals() which this method calls first.
    *
    * @param complete if true, handle completion & roation internally.
    */
    void setHandleSignals( bool handle );

    /**
    * Returns true if the completion object is deleted
    * upon this widget's destruction.
    *
    * See @ref setCompeltionObject and @ref enableCompletion
    * for details.
    *
    * @return true if the completion object
    */
    bool isCompletionObjectAutoDeleted() const { return m_bAutoDelCompObj; }

    /**
    * Sets the completion object for deletion upon this
    * widget's destruction.
    *
    * If the argument is set to true, the completion object
    * is deleted when this widget's destructor is called.
    *
    * @param autoDelete if true, delete completion object on destruction.
    */
    void setAutoDeleteCompletionObject( bool autoDelete ) { m_bAutoDelCompObj = autoDelete; }


    /**
    * Disables (temporarily) this widget's ability to emit
    * the rotation and completion signals.
    *
    * Invoking this function will cause the completion &
    * rotation signals not to be emitted.  Note that this
    * also disbales any internal handling of these signals.
    * However, unlike @ref setCompletionObject object,
    * disabling the emition of the signals through this
    * method does not delete the comlpetion object.
    */
    void disableSignals() { m_bEmitSignals = false; }

    /**
    * Enables the widget's ability to emit completion signals.
    *
    * Note that there is no need to invoke this function by
    * default.  When a completion object is created through
    * completionObject() or setCompletionObject(), these
    * signals are automatically actiavted.  Only call this
    * functions if you disbaled them manually.
    */
    void enableSignals()  { if( !m_bEmitSignals ) m_bEmitSignals = true; }

    /**
    * Returns true if the object handles the signals
    *
    * @return true if this signals are handled internally.
    */
    bool handleSignals() const { return m_bHandleSignals; }

    /**
    * Sets the type of completion to be used.
    *
    * The completion modes supported are those defined in
    * @ref KGlobal.  See below.
    *
    * @param mode Completion type:
    *   @li CompletionNone:  Disables completion feature.
    *   @li CompletionAuto:  Attempts to find a match &
    *                        fills-in the remaining text.
    *   @li CompletionMan:   Acts the same as the above
    *                        except the action has to be
    *                        manually triggered through
    *                        pre-defined completion key.
    *   @li CompletionShell: Mimics the completion feature
    *                        found in typcial *nix shell
    *                        enviornments.
    */
    virtual void setCompletionMode( KGlobal::Completion mode );

    /**
    * Retrieves the current completion mode.
    *
    * The return values are of type @ref KGlobal::Completion.
    * See @ref setCompletionMode for details.
    *
    * @return the completion mode.
    */
    KGlobal::Completion completionMode() const { return m_iCompletionMode; }

    /**
    * Sets the key-binding(s) to be used for rotating through
    * a list to find the next match.
    *
    * When this key is activated by the user a @ref rotateDown
    * signal will be emitted.  If no value is supplied for
    * rDnkey or it is set to 0, then the completion key will
    * be defaulted to the global setting.  This method returns
    * false if rDnkey is negative or the supplied key-binding
    * conflicts with either @ref completion or @ref rotateUp keys.
    *
    * @param rDnkey key-binding used to rotate up in a list.
    * @return true if key-binding can successfully be set.
    */
    bool setRotateDownKey( int rDnKey = 0 );

    /**
    * Sets the key-binding to be used for rotating through a
    * list to find the previous match.
    *
    * When this key is activated by the user a @ref rotateUp
    * signal will be emitted.  If no value is supplied for
    * rUpkey or it is set to 0, then the completion key
    * will be defaulted to the global setting.  This method
    * returns false if rUpkey is negative or the supplied
    * key-binding conflicts with either @ref completion or
    * @ref rotateDown keys.
    *
    * @param rUpkey key-binding used to rotate down in a list.
    * @return  true if key-binding can successfully be set.
    */
    bool setRotateUpKey( int rUpKey = 0 );

    /**
    * Sets the key-binding to be used for the two manual
    * completion types: CompletionMan and CompletionShell.
    *
    * This function expects the value of the modifier keys
    * (Shift, Ctrl, Alt), if present, to be @bf summed up
    * with actual key, ex: Qt::CTRL+Qt::Key_E.  If no value
    * is supplied for  ckey or it is set to 0, then the
    * completion key will be defaulted to the global setting.
    * This function returns true if the supplied key-binding
    * can be successfully assigned.
    *
    * Note that if  ckey is negative or the key-binding
    * conflicts with either @ref completion or @ref rotateDown
    * keys, this function will return false.  This method
    * will also always returns false if the widget is not
    * editable.
    *
    * @param ckey Key binding to use for completion
    * @return true if key-binding is successfully set.
    */
    bool setCompletionKey( int ckey = 0 );

    /**
    * Returns the key-binding used for completion.
    *
    * If the key binding contains modifier key(s), the @bf sum
    * of the key and the modifier will be returned. See also
    * @ref setCompletionKey.  Note that this method is only
    * useful when this widget is editable.  Otherwise this
    * method has no meaning.
    *
    * @return the key-binding used for rotating through a list.
    */
    int completionKey() const { return m_iCompletionKey; }

    /**
    * Returns the key-binding used for rotating up in a list.
    *
    * This methods returns the key used to iterate through a
    * list in the "UP" direction.  This is opposite to what
    * the @ref rotateDown key does.
    *
    * If the key binding contains modifier key(s), the SUM of
    * their values is returned.  See also @ref setRotateUpKey.
    *
    * @return the key-binding used for rotating up in a list.
    */
    int rotateUpKey() const { return m_iRotateUpKey; }

    /**
    * Returns the key-binding used for rotating down in a list.
    *
    * This methods returns the key used to iterate through a
    * list in the "DOWN" direction.  This is opposite to what
    * the @ref rotateDown key does.
    *
    * If the key binding contains modifier key(s), the SUM of
    * their values is returned.  See also @ref setRotateDownKey.
    *
    * @return the key-binding used for rotating down in a list.
    */
    int rotateDownKey() const { return m_iRotateDnKey; }

    /**
    * Sets this object to use global values for key-bindings.
    *
    * This method changes the values of the key bindings for
    * rotation and completion features to the default values
    * provided in KGlobal.
    *
    * By default this object uses the global key-bindings.
    * There is no need to call this method unless you have
    * locally modified the key bindings and want to revert
    * back.
    */
    void useGlobalSettings();


protected:

    /**
    * Method to be implemented by inheriting objects.
    *
    * This function must provide an implementation for
    * how signals are connected when they are handled
    * internally.  Since this is implementation dependent
    * it is left upto each inheriting object to decide.
    */
    virtual void connectSignals( bool handle ) const = 0;

    // Stores the completion key locally
    int m_iCompletionKey;
    // Stores the Rotate up key locally
    int m_iRotateUpKey;
    // Stores the Rotate down key locally
    int m_iRotateDnKey;

    // Flag that determined whether the completion object
    // should be deleted when this object is destroyed.
    bool m_bAutoDelCompObj;
    // Determines whether this widget handles completion signals
    // internally or not
    bool m_bHandleSignals;
    // Determines whether this widget fires rotation signals
    bool m_bEmitSignals;

    // Stores the completion mode locally.
    KGlobal::Completion m_iCompletionMode;
    // Pointer to Completion object.
    KCompletion* m_pCompObj;

    KCompletionPrivate *d;
};

#endif // KCOMPLETION_H
