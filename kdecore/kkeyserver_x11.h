/*
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

    Win32 port:
    Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef _KKEYSERVER_X11_H
#define _KKEYSERVER_X11_H

#include "kshortcut.h"
#include "kkeynative.h"

/**
 * A collection of functions for the conversion of key presses and
 * their modifiers from the window system specific format
 * to the generic format and vice-versa.
 */
namespace KKeyServer
{
	/**
	 * Supplement enum KKey::ModFlag
	 * @since 3.1
	 */
	enum ExtraModFlag { MODE_SWITCH = 0x2000 };

	/**
	 * Represents a key symbol.
	 * @see KKey
	 * @see KKeyServer
	 */
	struct KDECORE_EXPORT Sym
	{
	 public:
		/// the actual value of the symbol
		uint m_sym;

		/// Creates a null symbol.
		Sym()
			{ m_sym = 0; }
		/**
		 * Creates asymbol with the given value.
		 * @param sym the value
		 */
		Sym( uint sym )
			{ m_sym = sym; }
		/**
		 * Creates a symbol from the given string description.
		 * @param s the description of the symbol
		 * @see toString()
		 */
		Sym( const QString& s )
			{ init( s ); }

		/**
		 * Initializes the symbol with the given Qt key code.
		 * @param keyQt the qt key code
		 * @return true if successful, false otherwise
		 * @see Qt::Key
		 */
		bool initQt( int keyQt );

		/**
		 * Initializes the key with the given string description.
		 * @param s the string description
		 * @return true if successful, false otherwise
		 * @see toString()
		 */
		bool init( const QString &s );

		/**
		 * Returns the qt key code of the symbol.
		 * @return the qt key code
		 */
		int qt() const;

		/**
		 * @internal
		 */
		QString toStringInternal() const;

		/**
		 * Returns the string representation of the symbol.
		 * @return the string representation of the symbol
		 */
		QString toString() const;

		/**
		 * Returns the mods that are required for this symbol as
		 * ORed KKey::ModFlag's. For example, Break requires a
		 * Ctrl to be valid.
		 * @return the required KKey::ModFlag's
		 * @see KKey::ModFlag
		 */
		uint getModsRequired() const;

		/**
		 * TODO: please find out what this method does and document it
		 */
		uint getSymVariation() const;

		/**
		 * Casts the symbol to its integer representation.
		 */
		operator uint() const { return m_sym; }

		/**
		 * Overloaded operator to convert ints to Sym.
		 */
		Sym& operator =( uint sym ) { m_sym = sym; return *this; }

	 private:
		QString toString( bool bUserSpace ) const;

		static void capitalizeKeyname( QString& );
	};

	/**
	 * Represents a key press.
	 * @see KKey
	 */
	struct KDECORE_EXPORT Key
	{
		/// Code for native Keys in Qt
		enum { CODE_FOR_QT = 256 };

		/// The code of the key
		uint m_code;

		/// The modifiers of the key
		uint m_mod;

		/// The symbol of the key
		uint m_sym;

		/**
		 * Initializes the key with a KKey.
		 * @param key the key to get the data from
		 * @param bQt true to take the Qt keycode, false
		 *            for the native key code
		 * @see Key
		 * @see KKeyNative
		 */
		bool init( const KKey& key, bool bQt );

		/**
		 * Checks whether the key code is a native code.
		 * @return true if native code of the window system,
		 *         false if it is a Qt keycode
		 * @see Key
		 * @see KKeyNative
		 */
		bool isNative() const { return m_code != CODE_FOR_QT; }

		/**
		 * Returns the code of the key.
		 * @return the code of the key
		 */
		uint code() const { return m_code; }

		/**
		 * Returns the modifiers of the key.
		 * @return the modifiers of the key
		 */
		uint mod() const { return m_mod; }

		/**
		 * Returns the symbol of the key.
		 * @return the symbol of the key
		 */
		uint sym() const { return m_sym; }

		/**
		 * Returns the qt key code.
		 * @return the qt key code
		 */
		int keyCodeQt() const { return (int) m_sym; }

		/**
		 * Sets the qt key code.
		 * @param keyQt the qt key code
		 */
		void setKeycodeQt( int keyQt )
			{ m_code = CODE_FOR_QT; m_sym = keyQt; }

		/**
		 * Initializes this key with a KKeyNative.
		 * @return this key
		 */
		Key& operator =( const KKeyNative& key );

		/**
		 * Compares this key with the given Key object. Returns a
		 * negative number if the given Key is larger, 0 if they
		 * are equal and a positive number this Key is larger. The
		 * returned value is the difference between the symbol, modifier
		 * or code, whatever is non-zero first.
		 *
		 * @param key the key to compare with this key
		 * @return a negative number if the given Key is larger, 0 if
		 * they are equal and a positive number this Key is larger
		 */
		int compare( const Key& key ) const;

		/**
		 * Compares the symbol, modifiers and code of both keys.
		 * @see compare()
		 */
		bool operator ==( const Key& b ) const
			{ return compare( b ) == 0; }

		/**
		 * Compares the symbol, modifiers and code of both keys.
		 * @see compare()
		 */
		bool operator <( const Key& b ) const
			{ return compare( b ) < 0; }

		/**
		 * Converts this Key to a KKey.
		 * @return the KKey
		 */
		KKey key() const;
	};

	/**
	 * TODO: please document this class
	 */
	struct KDECORE_EXPORT Variations
	{
		enum { MAX_VARIATIONS = 4 };

		Key m_rgkey[MAX_VARIATIONS];
		uint m_nVariations;

		Variations() { m_nVariations = 0; }

		void init( const KKey&, bool bQt );

		uint count() const { return m_nVariations; }
		const Key& key( uint i ) const { return m_rgkey[i]; }
	};

	/// TODO: please document
	KDECORE_EXPORT bool initializeMods();

#ifdef KDE3_SUPPORT
	/**
	 * Returns the equivalent X modifier mask of the given modifier flag.
	 * @param modFlag the generic flags to check
	 * @return the window system specific flags
	 */
	KDECORE_EXPORT uint modX( KKey::ModFlag modFlag );
#endif

	/**
	 * Returns true if the current keyboard layout supports the Win key.
	 * Specifically, whether the Super or Meta keys are assigned to an X modifier.
	 * @return true if the keyboard has a Win key
	 * @see modXWin()
	 */
	KDECORE_EXPORT bool keyboardHasWinKey();

	/**
	 * Returns the X11 Shift modifier mask/flag.
	 * @return the X11 Shift modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDECORE_EXPORT uint modXShift();

	/**
	 * Returns the X11 Lock modifier mask/flag.
	 * @return the X11 Lock modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDECORE_EXPORT uint modXLock();

	/**
	 * Returns the X11 Ctrl modifier mask/flag.
	 * @return the X11 Ctrl modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDECORE_EXPORT uint modXCtrl();

	/**
	 * Returns the X11 Alt (Mod1) modifier mask/flag.
	 * @return the X11 Alt (Mod1) modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDECORE_EXPORT uint modXAlt();

	/**
	 * Returns the X11 NumLock modifier mask/flag.
	 * @return the X11 NumLock modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDECORE_EXPORT uint modXNumLock();

	/**
	 * Returns the X11 Win (Mod3) modifier mask/flag.
	 * @return the X11 Win (Mod3) modifier mask/flag.
	 * @see keyboardHasWinKey()
	 * @see accelModMaskX()
	 */
	KDECORE_EXPORT uint modXWin();
	KDECORE_EXPORT uint modXMeta();

	/**
	 * Returns the X11 ScrollLock modifier mask/flag.
	 * @return the X11 ScrollLock modifier mask/flag.
	 * @see accelModMaskX()
	 */
	KDECORE_EXPORT uint modXScrollLock();

	/**
	 * Returns the X11 Mode_switch modifier mask/flag.
	 * @return the X11 Mode_switch modifier mask/flag.
	 * @see accelModMaskX()
	 * @since 3.5
	 */
	KDECORE_EXPORT uint modXModeSwitch();

	/**
	 * Returns bitwise OR'ed mask containing Shift, Ctrl, Alt, and
	 * Win (if available).
	 * @see modXShift()
	 * @see modXLock()
	 * @see modXCtrl()
	 * @see modXAlt()
	 * @see modXNumLock()
	 * @see modXWin()
	 * @see modXScrollLock()
	 */
	KDECORE_EXPORT uint accelModMaskX();

	/**
	 * Extracts the symbol from the given Qt key and
	 * converts it to a symbol.
	 * @param keyQt the qt key code
	 * @param sym if successful, the symbol will be written here
	 * @return true if successful, false otherwise
	 * @see Key
	 * @see Sym
	 */
	KDECORE_EXPORT bool keyQtToSym( int keyQt, uint& sym );

	/**
	 * Extracts the modifiers from the given Qt key and
	 * converts them in a mask of ORed KKey::ModFlag modifiers.
	 * @param keyQt the qt key code
	 * @param mod if successful, the modifiers will be written here
	 * @return true if successful, false otherwise
	 * @see Key
	 */
	KDECORE_EXPORT bool keyQtToMod( int keyQt, uint& mod );

	/**
	 * Converts the given symbol to a Qt key code.
	 * @param sym the symbol
	 * @param keyQt if successful, the qt key code will be written here
	 * @return true if successful, false otherwise
	 * @see Key
	 * @see Sym
	 */
	KDECORE_EXPORT bool symToKeyQt( uint sym, int& keyQt );

	/**
	 * Converts the mask of ORed KKey::ModFlag modifiers to
	 * a mask of ORed Qt key code modifiers.
	 * @param mod the mask of KKey::ModFlag modifiers
	 * @param modQt the mask of Qt key code modifiers will be written here,
	 *        if successful
	 * @return true if successful, false otherwise
	 * @see Key
	 * @see KKey
	 */
	KDECORE_EXPORT bool modToModQt( uint mod, int& modQt );

	/**
	 * Converts the mask of ORed KKey::ModFlag modifiers to
	 * a mask of ORed X11 modifiers.
	 * @param mod the mask of KKey::ModFlag modifiers
	 * @param modX the mask of X11 modifiers will be written here,
	 *        if successful
	 * @return true if successful, false otherwise
	 * @see KKey
	 */
	KDECORE_EXPORT bool modToModX( uint mod, uint& modX );

	/**
	 * Converts the mask of ORed X11 modifiers to
	 * a mask of ORed Qt key code modifiers.
	 * @param modX the mask of X11 modifiers
	 * @param modQt the mask of Qt key code modifiers will be written here
	 *        if successful
	 * @return true if successful, false otherwise
	 * @see Key
	 */
	//wrapped for win32
	KDECORE_EXPORT bool modXToModQt( uint modX, int& modQt );

	/**
	 * Converts the Qt-compatible button state to x11 modifier.
	 */
	KDECORE_EXPORT int qtButtonStateToMod( Qt::ButtonState s );

	/**
	 * Converts the mask of ORed X11 modifiers to
	 * a mask of ORed KKey::ModFlag modifiers.
	 * @param modX the mask of X11 modifiers
	 * @param mod the mask of KKey::ModFlag modifiers will be written here,
	 *        if successful
	 * @return true if successful, false otherwise
	 * @see KKey
	 */
	KDECORE_EXPORT bool modXToMod( uint modX, uint& mod );

	/**
	 * Converts a X11 key code and a mask of ORed X11 modifiers
	 * into a X11 symbol.
	 * converts it to a symbol.
	 * @param codeX the X11 key code
	 * @param modX the mask of ORed X11 modifiers
	 * @param symX if successful, the X11 symbol will be written here
	 * @return true if successful, false otherwise
	 * @see Key
	 * @see Sym
	 */
	KDECORE_EXPORT bool codeXToSym( uchar codeX, uint modX, uint& symX );

	/**
	 * @internal
	 */
	KDECORE_EXPORT QString modToStringInternal( uint mod );

	/**
	 * Converts the mask of ORed KKey::ModFlag modifiers to a
	 * user-readable string.
	 * @param mod the mask of ORed KKey::ModFlag modifiers
	 * @return the user-readable string
	 */
	KDECORE_EXPORT QString modToStringUser( uint mod );

	/**
	 * Converts the modifier given as user-readable string
         * to KKey::ModFlag modifier, or 0.
         * @internal
         * @since 3.5
	 */
	KDECORE_EXPORT uint stringUserToMod( const QString& mod );

	/**
	 * @internal
	 * Unimplemented?
	 */
	KDECORE_EXPORT bool stringToSymMod( const QString&, uint& sym, uint& mod );

	/**
	 * @internal
	 * Unimplemented?
	 */
	KDECORE_EXPORT void keyQtToKeyX( uint keyCombQt, unsigned char *pKeyCodeX, uint *pKeySymX, uint *pKeyModX );
}

#endif // !_KKEYSERVER_X11_H
