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

#ifndef K3KEYSEQUENCE_H
#define K3KEYSEQUENCE_H

#include "kshortcut.h"

/**
* A K3KeySequence object holds a sequence of up to 4 keys.
* Ex: Ctrl+X,I
* @see KKey
* @see KShortcut
*/

class KDE3SUPPORT_EXPORT K3KeySequence
{
 public:
        /// Defines the maximum length of the key sequence
        enum { MAX_KEYS = 4 };

	/**
	 * Create a new null key sequence.
	 * @see isNull()
	 * @see null()
	 * @see clear()
	 */
	K3KeySequence();

	/**
	 * Copies the given qt key sequence.
	 * @param keySeq the qt key sequence to copy
	 */
	K3KeySequence( const QKeySequence& keySeq );

	/**
	 * Create a new key sequence that only contains the given key.
	 * @param key the key to add
	 */
	K3KeySequence( const KKey& key );

	/**
	 * Create a new key sequence that only contains the given key.
	 * @param key the key to add
	 */
	K3KeySequence( const KKeyNative& key );

	/**
	 * Copies the given key sequence.
	 * @param keySeq the key sequence to copy
	 */
	K3KeySequence( const K3KeySequence& keySeq );

	/**
	 * Creates a new key sequence that contains the given key sequence.
	 * The description consists of comma-separated keys as
	 * required by KKey::KKey(const QString&).
	 * @param keySeq the description of the key
	 * @see KKeyServer::Sym::init()
	 * @see KKey::KKey(const QString&)
	 */
	K3KeySequence( const QString& keySeq );

	~K3KeySequence();

	/**
	 * Clears the key sequence. The key sequence is null after calling this
	 * function.
	 * @see isNull()
	 */
	void clear();

	/**
	 * Copies the given qt key sequence over this key sequence.
	 * @param keySeq the qt key sequence to copy
	 * @return true if successful, false otherwise
	 */
	bool init( const QKeySequence& keySeq );

	/**
	 * Initializes the key sequence to only contain the given key.
	 * @param key the key to set
	 * @return true if successful, false otherwise
	 */
	bool init( const KKey& key );

	/**
	 * Initializes the key sequence to only contain the given key.
	 * @param key the key to set
	 * @return true if successful, false otherwise
	 */
	bool init( const KKeyNative& key );

	/**
	 * Copies the given key sequence over this key sequence.
	 * @param keySeq the key sequence to copy
	 * @return true if successful, false otherwise
	 */
	bool init( const K3KeySequence& keySeq );

	/**
	 * Initializes this key sequence to contain the given key sequence.
	 * The description consists of comma-separated keys as
	 * required by KKey::KKey(const QString&).
	 * @param key the description of the key
	 * @return true if successful, false otherwise
	 * @see KKeyServer::Sym::init()
	 * @see KKey::KKey(const QString&)
	 */
	bool init( const QString& key );

	/**
	 * Copy the given key sequence into this sequence.
	 */
	K3KeySequence& operator =( const K3KeySequence& seq )
		{ init( seq ); return *this; }

	/**
	 * Returns the number of key strokes of this sequence.
	 * @return the number of key strokes
	 * @see MAX_KEYS
	 */
	uint count() const;

	/**
	 * Return the @p i'th key of this sequence, or a null key if there
	 * are less then i keys.
	 * @param i the key to retrieve
	 * @return the @p i'th key, or KKey::null() if there are less
	 *         than i keys
	 * @see MAX_KEYS
	 */
	const KKey& key( uint i ) const;

	/**
	 * @internal
	 */
	bool isTriggerOnRelease() const;

	/**
	 * Sets the @p i'th key of the sequence. You can not introduce gaps
	 * in a sequence, so you must use an @p i <= count(). Also note that
	 * the maximum length of a key sequence is MAX_KEYS.
	 * @param i the position of the new key (<= count(), <= MAX_KEYS)
	 * @param key the key to set
	 * @return true if successful, false otherwise
	 */
	bool setKey( uint i, const KKey& key );

	/**
	 * Returns true if the key sequence is null (after clear() or empty
	 * constructor).
	 * @return true if the key sequence is null
	 * @see clear()
	 * @see null()
	 */
	bool isNull() const;

	/**
	 * Returns true if this key sequence begins with the given sequence.
	 * @param keySeq the key sequence to search
	 * @return true if this key sequence begins with the given sequence
	 */
	bool startsWith( const K3KeySequence& keySeq ) const;

	/**
	 * Compares this object with the given key sequence. Returns a negative
	 * number if the given K3KeySequence is larger, 0 if they are equal and
	 * a positive number this K3KeySequence is larger. Key sequences are
	 * compared by comparing the individual keys, starting from the beginning
	 * until an unequal key has been found. If a sequence contains more
	 * keys, it is considered larger.
	 * @param keySeq the key sequence to compare to
	 * @return a negative number if the given K3KeySequence is larger, 0 if
	 * they are equal and a positive number this K3KeySequence is larger
	 * @see KKey::sequence
	 */
	int compare( const K3KeySequence& keySeq ) const;

	/**
	 * Compares the keys of both sequences.
	 * @see compare()
	 */
	bool operator == ( const K3KeySequence& seq ) const
		{ return compare( seq ) == 0; }

	/**
	 * Compares the keys of both sequences.
	 * @see compare()
	 */
	bool operator != ( const K3KeySequence& seq ) const
		{ return compare( seq ) != 0; }

	/**
	 * Compares the keys of both sequences.
	 * @see compare()
	 */
	bool operator < ( const K3KeySequence& seq ) const
		{ return compare( seq ) < 0; }
	// TODO: consider adding Qt::SequenceMatch matches(...) methods for QKeySequence equivalence

	/**
	 * Converts this key sequence to a QKeySequence.
	 * @return the QKeySequence
	 */
	QKeySequence qt() const;

	/**
	 * Returns the qt key code of the first key.
	 * @return the qt key code of the first key
	 * @see Qt::Key
	 * @see KKey::keyCodeQt()
	 */
	int keyCodeQt() const;

	/**
	 * Returns the key sequence as a number of key presses as
	 * returned by KKey::toString(), separated by commas.
	 * @return the string represenation of this key sequence
	 * @see KKey::toString()
	 */
	QString toString() const;

	/**
	 * @internal
	 */
	QString toStringInternal() const;

	/**
	 * Returns a null key sequence.
	 * @return the null key sequence
	 * @see isNull()
	 * @see clear()
	 */
	static K3KeySequence& null();

 protected:
	uchar m_nKeys;
	uchar m_bTriggerOnRelease;
	KKey m_rgkey[MAX_KEYS];

 private:
	class K3KeySequencePrivate* d;
	friend class KKeyNative;
};

#endif
