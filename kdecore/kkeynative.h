/* This file is part of the KDE libraries
    Copyright (C) 2002 Ellis Whitehead <ellis@kde.org>

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

#ifndef __KKEYNATIVE_H
#define __KKEYNATIVE_H

#include <kshortcut.h>

typedef union _XEvent XEvent;

class KKeyNativePrivate;
/** 
 * Representation of a key in the format native of the windowing system (eg. X11).
 * @see KKey
 */
class KKeyNative
{
 public:
	/**
	 * Creates a new null KKey.
	 * @see clear()
	 * @see isNull()
	 * @see null()
	 */
	KKeyNative();

	/**
	 * Extracts a new native key from the given xevent.
	 * @param xevent the XEvent that contains the key
	 */
	KKeyNative( const XEvent* xevent );

	/**
	 * Creates a new native key for the given KKey code.
	 * @param key the @ref KKey that contains the generic key
	 */
	KKeyNative( const KKey& key );

	/**
	 * Copy constructor.
	 */
	KKeyNative( const KKeyNative& );

	/**
	 * @internal
	 */
	KKeyNative( uint code, uint mod, uint sym );
	~KKeyNative();

	/**
	 * Clears the key. The key is null after calling this function.
	 * @see isNull()
	 */
	void clear();

	/**
	 * Initializes the native key by extracting the information
	 * from the given xevent.
	 * @param xevent the XEvent that contains the key
	 * @return true if succesful, false otherwise
	 */
	bool init( const XEvent* xevent );

	/**
	 * Creates a new native key for the given KKey code.
	 * @param key the @ref KKey that contains the generic key
	 * @return true if succesful, false otherwise
	 */
	bool init( const KKey& key );

	/**
	 * Copies the given key into this key.
	 * @param key the key to copy
	 * @return true if succesful, false otherwise
	 */
	bool init( const KKeyNative& key );

	/**
	 * Copies the given key into this key.
	 * @param key the key to copy
	 * @return this key
	 */	
	KKeyNative& operator =( const KKeyNative& key )
		{ init( key ); return *this; }

	/**
	 * Returns the qt key code.
	 * @return the qt key code or 0 if there is no key set.
	 * @see Qt::Key
	 */
	
	int keyCodeQt() const;

	/**
	 * Returns the KKey representation of this key.
	 * @return the KKey representation
	 */
	KKey key() const;

	/**
	 * Converts this key to its KKey representation.
	 * @return the KKey representation
	 * @see key()
	 */
	operator KKey() const     { return key(); }

	/**
	 * The native keycode of the key.
	 * @return the native keycode
	 */
	uint code() const;

	/**
	 * The native modifier flags of the key.
	 * @return the native modifier flags
	 */
	uint mod() const;

	/**
	 * The native symbol (KeySym) of the key.
	 * @return the native symbol (KeySym)
	 */
	uint sym() const;

	/**
	 * Returns true if the key is null (after @ref clear() or empty
	 * constructor).
	 * @return true if the key is null
	 * @see clear()
	 * @see null()
	 */
	bool isNull() const;

	/**
	 * Compares this key with the given KKeyNative object. Returns a 
	 * negative number if the given KKeyNative is larger, 0 if they 
	 * are equal and a positive number this KKeyNative is larger. The 
	 * returned value is the difference between the symbol, modifier
	 * or code, whatever is non-zero first.
	 *
	 * @param key the key to compare with this key
	 * @return a negative number if the given KKeyNative is larger, 0 if 
	 * they are equal and a positive number this KKeyNative is larger
	 */
	int compare( const KKeyNative& key ) const;

	/**
	 * Compares the symbol, modifiers and code of both keys.
	 * @see compare()
	 */
	bool operator == ( const KKeyNative& key ) const
		{ return compare( key ) == 0; }

	/**
	 * Compares the symbol, modifiers and code of both keys.
	 * @see compare()
	 */
	bool operator != ( const KKeyNative& key ) const
		{ return compare( key ) != 0; }

	/**
	 * Compares the symbol, modifiers and code of both keys.
	 * @see compare()
	 */
	bool operator < ( const KKeyNative& key ) const
		{ return compare( key ) < 0; }

	/**
	 * Returns a null key.
	 * @return the null key
	 * @see isNull()
	 * @see clear()
	 */
	static KKeyNative& null();

	// General query functions. //
	/**
	 * Checks whether the keyboard has a Win key.
	 * @return true if the keyboard has a Win key
	 */
	static bool keyboardHasWinKey();

	/**
	 * Returns the equivalent X modifier mask of the given modifier flag.
	 * @param modFlag the mod flags to test
	 * @return the equivalent native flags of the window system
	 */
	static uint modX( KKey::ModFlag modFlag );

	/**
	 * Returns bitwise OR'ed mask containing Shift, Ctrl, Alt, and
	 * Win (if available).
	 * @return the mask of Shift, Ctrl, Alt and Win.
	 */
	static uint accelModMaskX();

 private:
	uint m_code, m_mod, m_sym;
	KKeyNativePrivate* d;
};

#endif // !__KKEYNATIVE_H
