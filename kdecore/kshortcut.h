/*  This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

/**
 * @file kshortcut.h
 * Defines platform-independent classes for keyboard shortcut handling.
 */

#ifndef KSHORTCUT_H
#define KSHORTCUT_H

#include "kdelibs_export.h"

#include <QKeySequence>
#include <QSharedDataPointer>

/**
* @short Represents a keyboard shortcut
*
* The KShortcut class is used to represent a keyboard shortcut to an action.
* A shortcut is normally a single key with modifiers, such as Ctrl+V.
* A KShortcut object may also contain multiple alternate keys which will also
* activate the action it's associated to, as long as no other actions have
* defined that key as their primary key.  Ex: Ctrl+V;Shift+Insert.
*
* This can be used to add additional accelerators to a KAction.  For example,
* the below code binds the escape key to the close action.
*
* \code
*  KAction *closeAction = KStdAction::close(this, SLOT( close() ), actionCollection());
*  KShortcut closeShortcut = closeAction->shortcut();
*  closeShortcut.append(Qt::Key_Escape);
*  closeAction->setShortcut(closeShortcut);
* \endcode
*
* \note This class is implicitly shared, and all functions are reentrant.
*
* \todo make constructors explicit
* \todo needs serious cleaning up (rodda@kde.org)
*/
class KDECORE_EXPORT KShortcut
{
public:
	/**
	 * Creates a new null shortcut.
	 * @see null()
	 * @see isNull()
	 * @see clear()
	 */
	KShortcut();

	/**
	 * Creates a new shortcut with the given Qt key code
	 * as the only key sequence.
	 * @param keyQt the qt keycode
	 * @see Qt::Key
	 */
	KShortcut( int keyQt );

	/**
	 * Creates a new shortcut that contains only the given qt key
	 * sequence.
	 * @param keySeq the qt key sequence to add
	 */
	KShortcut( const QKeySequence& keySeq );

	/**
	 * Creates a new shortcut that contains only the given qt key
	 * sequences.
	 * @param keySeq1 the first sequence
	 * @param keySeq2 the second sequence
	 */
	KShortcut( const QKeySequence& keySeq1, const QKeySequence& keySeq2 );

	/**
	 * Copies the given shortcut.
	 * @param shortcut the shortcut to add
	 */
	KShortcut( const KShortcut& shortcut );

	/**
	 * Creates a new key sequence that contains the given key sequence.
	 * The description consists of semicolon-separated keys as
	 * used in QKeySequence::fromString(const QString&, SequenceFormat).
	 * @param shortcut the description of the key
	 * @see QKeySequence::fromString(const QString&, SequenceFormat).
	 */
	KShortcut( const QString& shortcut );
	
	/**
	 * Destructor.
	 */
	~KShortcut();

	/** @name Initialization methods */
	/** @{ */

	/**
	 * Clears the shortcut. The shortcut is null after calling this
	 * function.
	 * @see isNull()
	 */
	void clear();

	/**
	 * Initializes the shortcut with the given Qt key code
	 * as the only key sequence.
	 * @param keyQt the qt keycode
	 * @see Qt::Key
	 */
	bool init( int keyQt );

	/**
	 * Initializes the shortcut with the given qt key sequence.
	 * @param keySeq the qt key sequence to add
	 */
	bool init( const QKeySequence& keySeq );

	/**
	 * Copies the given shortcut.
	 * @param shortcut the shortcut to add
	 */
	bool init( const KShortcut& shortcut );

	/**
	 * Initializes the key sequence with the given key sequence.
	 * The description consists of semicolon-separated keys as
	 * used in QKeySequence::fromString(const QString&, SequenceFormat).
	 * @param shortcut the description of the key
	 * @see QKeySequence::fromString(const QString&, SequenceFormat).
	 */
	bool init( const QString& shortcut );

	/**
	 * Copies the given shortcut over this shortcut.
	 * @param cut the shortcut to copy
	 */
	KShortcut& operator =( const KShortcut& cut )
		{ init( cut ); return *this; }

	/** @} */
	/** @name Query methods */
	/** @{ */

	/**
	 * Returns the number of sequences that are in this
	 * shortcut.
	 * @return the number of sequences
	 * MAX_SEQUENCES
	 */
	int count() const;

	/**
	 * Returns the @p i'th key sequence of this shortcut.
	 * @param i the number of the key sequence to retrieve
	 * @return the @p i'th sequence or a null QKeySequence if
	 *         there are less than @p i key sequences
	 * @see MAX_SEQUENCES
	 */
	const QKeySequence seq( int i ) const;
	
	/**
	 * Returns all key sequences in order.
	 */
	const QList<QKeySequence>& sequences() const;

	/**
	 * Returns the key code of the first key sequence, or
	 * null if there is no first key sequence.
	 * @return the key code of the first sequence's first key
	 * @see Qt::Key
	 */
	int keyQt() const;

	/**
	 * Returns @c true if the shortcut is null (after clear() or empty
	 * constructor).
	 * @return @c true if the shortcut is null, @c false otherwise
	 * @see clear()
	 * @see null()
	 */
	bool isNull() const;

	/**
	 * Checks whether this shortcut contains a sequence that starts
	 * with the given key.
	 * @param keyQt the key to check
	 * @return @c true if a key sequence starts with the key, @c false
	 *         otherwise
	 */
	bool contains( int keyQt ) const;

	/**
	 * Checks whether this shortcut contains the given sequence.
	 * @param keySeq the key sequence to check
	 * @return @c true if the shortcut has the given key sequence,
	 *         @c false otherwise
	 */
	bool contains( const QKeySequence& keySeq ) const;

	/** @} */
	/** @name Comparison methods */
	/** @{ */

	/**
	 * Compares this object with the given shortcut. Returns a negative
	 * number if the given shortcut is larger, 0 if they are equal and
	 * a positive number this shortcut is larger. Shortcuts are
	 * compared by comparing the individual key sequences, starting from the
	 * beginning until an unequal key sequences has been found. If a shortcut
	 * contains more key sequences, it is considered larger.
	 * @param shortcut the shortcut to compare to
	 * @return a negative number if the given KShortcut is larger, 0 if
	 * they are equal and a positive number this KShortcut is larger
	 */
	int compare( const KShortcut& shortcut ) const;

	/**
	 * Compares the sequences of both shortcuts.
	 * @param cut the shortcut to compare to
	 * @return @c true, if both shortcuts are equal, @c false otherwise
	 * @see compare()
	 */
	bool operator == ( const KShortcut& cut ) const;

	/**
	 * Compares the sequences of both shortcuts.
	 * @param cut the shortcut to compare to
	 * @return @c true, if the shortcuts are not equal, @c false otherwise
	 * @see compare()
	 */
	bool operator != ( const KShortcut& cut ) const
		{ return !operator==( cut ); }

	/**
	 * Compares the sequences of both shortcuts.
	 * @param cut the shortcut to compare to
	 * @return @c true, if @c this is smaller than @p cut, @c false otherwise
	 * @see compare()
	 */
	bool operator < ( const KShortcut& cut ) const
		{ return compare( cut ) < 0; }

	/** @} */
	/** @name Operation methods */
	/** @{ */

	/**
	 * Sets the @p i 'th key sequence of the shortcut. You can not introduce
	 * gaps in the list of sequences, so you must use an @p i <= count().
	 * Also note that the maximum number of key sequences is MAX_SEQUENCES.
	 * @param i the position of the new key sequence. if i is at or greater than
	 *        count(), the sequence will be appended instead.
	 * @param keySeq the key sequence to set
	 */
	void setSeq( int i, const QKeySequence& keySeq );

	/**
	 * Appends the given key sequence.  This sets it as either the keysequence or
	 * the alternate keysequence.  If the shortcut already has MAX_SEQUENCES
	 * sequences then this call does nothing, and returns false.
	 *
	 * @param keySeq the key sequence to add
	 * @see setSeq()
	*/
	void append( const QKeySequence& keySeq );

	/**
	 * Removes the given key sequence from this shortcut
	 * @param keySeq the key sequence to remove
	*/
	void remove( const QKeySequence& keySeq );

	/** @} */
	/** @name Conversion methods */
	/** @{ */

	/**
	 * Converts this shortcut to a key sequence. The first key sequence
	 * will be taken.
	 */
	operator QKeySequence () const;

	/**
	 * Returns a description of the shortcut as semicolon-separated
	 * key sequences, as returned by QKeySequence::toString().
	 * @return the string represenation of this shortcut
	 * @see QKeySequence::toString()
	 */
	QString toString() const;

	/// \internal
	QString toStringInternal( const KShortcut* pcutDefault = 0L ) const;

	/** @} */

	/**
	 * Returns a null shortcut.
	 * @return the null shortcut
	 * @see isNull()
	 * @see clear()
	 */
	static const KShortcut& null();

private:
	QSharedDataPointer<class KShortcutPrivate> d;
};

#endif // KSHORTCUT_H

