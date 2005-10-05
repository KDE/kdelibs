/*  This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

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

#ifndef K3SHORTCUT_H
#define K3SHORTCUT_H

#include "kshortcut.h"
#include "k3keysequence.h"

/**
* The K3Shortcut class is used to represent a keyboard shortcut to an action.
* A shortcut is normally a single key with modifiers, such as Ctrl+V.
* A K3Shortcut object may also contain an alternate key which will also
* activate the action it's associated to, as long as no other actions have
* defined that key as their primary key.  Ex: Ctrl+V;Shift+Insert.
*
* This can be used to add additional accelerators to a KAction.  For example,
* the below code binds the escape key to the close action.
*
* \code
*  KAction *closeAction = KStdAction::close( this, SLOT( close() ), actionCollection() );
*  K3Shortcut closeShortcut = closeAction->shortcut();
*  closeShortcut.append( KKey(Key_Escape));
*  closeAction->setShortcut(closeShortcut);
* \endcode
*
* Note that a shortcut cannot have more than 2 key combinations associated with it, so the above
* code would not do anything (and .append would return false) if the closeAction already had
* an key and alternate key.
* 
*/

class KDE3SUPPORT_EXPORT K3Shortcut
{
 public:
        /**
	 * The maximum number of key sequences that can be contained in
	 * a K3Shortcut.
         */
	enum { MAX_SEQUENCES = 2 };

	/**
	 * Creates a new null shortcut.
	 * @see null()
	 * @see isNull()
	 * @see clear()
	 */
	K3Shortcut();

	/**
	 * Creates a new shortcut with the given Qt key code
	 * as the only key sequence.
	 * @param keyQt the qt keycode
	 * @see Qt::Key
	 */
	K3Shortcut( int keyQt );

	/**
	 * Creates a new shortcut that contains only the given qt key
	 * sequence.
	 * @param keySeq the qt key sequence to add
	 */
	K3Shortcut( const QKeySequence& keySeq );

	/**
	 * Creates a new shortcut that contains only the given key
	 * in its only sequence.
	 * @param key the key to add
	 */
	K3Shortcut( const KKey& key );

	/**
	 * Creates a new shortcut that contains only the given key
	 * sequence.
	 * @param keySeq the key sequence to add
	 */
	K3Shortcut( const K3KeySequence& keySeq );

	/**
	 * Copies the given shortcut.
	 * @param shortcut the shortcut to add
	 */
	K3Shortcut( const K3Shortcut& shortcut );

	/**
	 * Creates a new key sequence that contains the given key sequence.
	 * The description consists of semicolon-separated keys as
	 * used in K3KeySequence::K3KeySequence(const QString&).
	 * @param shortcut the description of the key
	 * @see K3KeySequence::K3KeySequence(const QString&)
	 */
	K3Shortcut( const char* shortcut );

	/**
	 * Creates a new key sequence that contains the given key sequence.
	 * The description consists of semicolon-separated keys as
	 * used in K3KeySequence::K3KeySequence(const QString&).
	 * @param shortcut the description of the key
	 * @see K3KeySequence::K3KeySequence(const QString&)
	 */
	K3Shortcut( const QString& shortcut );
	~K3Shortcut();

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
	 * Initializes the shortcut with the given key as its only sequence.
	 * @param key the key to add
	 */
	bool init( const KKey& key );

	/**
	 * Initializes the shortcut with the given qt key sequence.
	 * @param keySeq the qt key sequence to add
	 */
	bool init( const K3KeySequence& keySeq );

	/**
	 * Copies the given shortcut.
	 * @param shortcut the shortcut to add
	 */
	bool init( const K3Shortcut& shortcut );

	/**
	 * Initializes the key sequence with the given key sequence.
	 * The description consists of semicolon-separated keys as
	 * used in K3KeySequence::K3KeySequence(const QString&).
	 * @param shortcut the description of the key
	 * @see K3KeySequence::K3KeySequence(const QString&)
	 */
	bool init( const QString& shortcut );

	/**
	 * Copies the given shortcut over this shortcut.
	 */
	K3Shortcut& operator =( const K3Shortcut& cut )
		{ init( cut ); return *this; }

	/**
	 * Returns the number of sequences that are in this
	 * shortcut.
	 * @return the number of sequences
	 * MAX_SEQUENCES
	 */
	uint count() const;

	/**
	 * Returns the @p i'th key sequence of this shortcut.
	 * @param i the number of the key sequence to retrieve
	 * @return the @p i'th sequence or K3KeySequence::null() if
	 *         there are less than @p i key sequences
	 * MAX_SEQUENCES
	 */
	const K3KeySequence& seq( uint i ) const;

	/**
	 * Returns the key code of the first key sequence, or
	 * null if there is no first key sequence.
	 * @return the key code of the first sequence's first key
	 * @see Qt::Key
	 * @see K3KeySequence::keyCodeQt()
	 */
	int keyCodeQt() const;

	/**
	 * Returns true if the shortcut is null (after clear() or empty
	 * constructor).
	 * @return true if the shortcut is null
	 * @see clear()
	 * @see null()
	 */
	bool isNull() const;

	/**
	 * Compares this object with the given shortcut. Returns a negative
	 * number if the given shortcut is larger, 0 if they are equal and
	 * a positive number this shortcut is larger. Shortcuts are
	 * compared by comparing the individual key sequences, starting from the
	 * beginning until an unequal key sequences has been found. If a shortcut
	 * contains more key sequences, it is considered larger.
	 * @param shortcut the shortcut to compare to
	 * @return a negative number if the given K3Shortcut is larger, 0 if
	 * they are equal and a positive number this K3Shortcut is larger
	 * @see KKey::compare()
	 * @see KKeyShortcut::compare()
	 */
	int compare( const K3Shortcut& shortcut ) const;

	/**
	 * Compares the sequences of both shortcuts.
	 * @see compare()
	 */
	bool operator == ( const K3Shortcut& cut ) const
		{ return compare( cut ) == 0; }

	/**
	 * Compares the sequences of both shortcuts.
	 * @see compare()
	 */
	bool operator != ( const K3Shortcut& cut ) const
		{ return compare( cut ) != 0; }

	/**
	 * Compares the sequences of both shortcuts.
	 * @see compare()
	 */
	bool operator < ( const K3Shortcut& cut ) const
		{ return compare( cut ) < 0; }

	/**
	 * Checks whether this shortcut contains a sequence that starts
	 * with the given key.
	 * @param key the key to check
	 * @return true if a key sequence starts with the key
	 */
	bool contains( const KKey& key ) const;

	/**
	 * Checks whether this shortcut contains a sequence that starts
	 * with the given key.
	 * @param key the key to check
	 * @return true if a key sequence starts with the key
	 */
	bool contains( const KKeyNative& key ) const;

	/**
	 * Checks whether this shortcut contains the given sequence.
	 * @param keySeq the key sequence to check
	 * @return true if the shortcut has the given key sequence
	 */
	bool contains( const K3KeySequence& keySeq ) const;

	/**
	 * Sets the @p i 'th key sequence of the shortcut. You can not introduce
	 * gaps in the list of sequences, so you must use an @p i <= count().
	 * Also note that the maximum number of key sequences is MAX_SEQUENCES.
	 * @param i the position of the new key sequence(0 <= i <= count(), 0 <= i < MAX_SEQUENCES)
	 * @param keySeq the key sequence to set
	 * @return true if successful, false otherwise
	 */
	bool setSeq( uint i, const K3KeySequence& keySeq );

	/**
	 * Appends the given key sequence.  This sets it as either the keysequence or
	 * the alternate keysequence.  If the shortcut already has MAX_SEQUENCES
	 * sequences then this call does nothing, and returns false.
	 *
	 * @param keySeq the key sequence to add
	 * @return true if successful, false otherwise
	 * @see setSeq()
	*/
	bool append( const K3KeySequence& keySeq );

	/**
	 * Removes the given key sequence from this shortcut
	 * @param keySeq the key sequence to remove
	 * @since 3.3
	*/
	void remove( const K3KeySequence& keySeq );

	/**
	 * Appends the given key
	 * @param spec the key to add
	 * @return true if successful, false otherwise
	 * @see setSeq()
	 * @see MAX_SEQUENCES
	 * @since 3.2
	*/
	bool append( const KKey& spec );

	/**
	 * Appends the sequences from the given shortcut.
	 * @param cut the shortcut to append
	 * @return true if successful, false otherwise
	 * @see MAX_SEQUENCES
	 * @since 3.2
	*/
	bool append( const K3Shortcut& cut );

	/**
	 * Converts this shortcut to a key sequence. The first key sequence
	 * will be taken.
	 */
	operator QKeySequence () const;

	/**
	 * Returns a description of the shortcut as semicolon-separated
	 * ket sequences, as returned by K3KeySequence::toString().
	 * @return the string represenation of this shortcut
	 * @see KKey::toString()
	 * @see K3KeySequence::toString()
	 */
	QString toString() const;

	/**
	 * @internal
	 */
	QString toStringInternal( const K3Shortcut* pcutDefault = 0 ) const;

	/**
	 * Returns a null shortcut.
	 * @return the null shortcut
	 * @see isNull()
	 * @see clear()
	 */
	static K3Shortcut& null();

 protected:
	uint m_nSeqs;
	K3KeySequence m_rgseq[MAX_SEQUENCES];

 private:
	class K3ShortcutPrivate* d;
	friend class KKeyNative;

#ifndef KDE_NO_COMPAT
 public:
	operator int () const    { return keyCodeQt(); }
#endif
};

#endif // KSHORTCUT_H

