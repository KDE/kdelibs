/* This file is part of the KDE libraries
    Copyright (C) 2001 Ellis Whitehead <kde@ellisw.net>

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

#ifndef _KKEYSEQUENCE_H
#define _KKEYSEQUENCE_H

#include <qkeysequence.h>
#include <qstring.h>
#include <qvaluevector.h>

class QKeyEvent;

class KKeySequence;
class KKeySequences;

// The following functions are platform dependent and are implemented in
//  their respective files (either kkey_x11.cpp or kkeysequence_emb.cpp).
//	KKeySequence( const QString& );
//	toString( KKeySequence::I18N bi18n = KKeySequence::I18N_Yes );
//	keyboardHasMetaKey();
//	void calcKeyQt();

class KKeySequence
{
 public:
	enum I18N { I18N_No, I18N_Yes };

	KKeySequence();
	KKeySequence( const KKeySequence& k );
	KKeySequence( const QKeySequence& k );
	KKeySequence( const QString& );
	KKeySequence( uint keyCombQt );
	KKeySequence( const QKeyEvent * );

	void clear();
	//KKeySequence& operator =( KKeySequence k );
	KKeySequence& operator =( uint keyCombQt );

	bool operator ==( const KKeySequence& k ) const { return compare( *this, k ) == 0; }
	bool operator !=( const KKeySequence& k ) const { return compare( *this, k ) != 0; }
	bool operator <( const KKeySequence& k ) const { return compare( *this, k ) < 0; }

	operator QKeySequence();
	int keyQt();
	/*uint key() const;
	uint sym() const;
	uint mod() const;
	uint state() const;*/
	bool isNull() const;

	QString toString( I18N bi18n = I18N_Yes ) const;

	/**
	 * Retrieve the key code corresponding to the string @p sKey or
	 * zero if the string is not recognized.
	 *
	 * The string must be something like "Shift+A",
	 * "Alt+Ctrl+F1" or "Backspace" for example. That is, the string
	 * must consist of a combination of the modifiers
	 * Meta, Alt, Ctrl, and Shift, Ctrl and Alt, followed by trigger key.
	 *
	 * N.B.: @p sKey must @em not be @ref i18n()'d!
	 */
	// This function is implemented in kkey_x11.cpp for X11
	//  and in kkeysequence_emb.cpp for Qt/Embedded.
	static KKeySequences stringToKeys( QString );

 // Config Functions

	// When bUseFourModifierKeys is on (setting: Global|Keyboard|Use Four Modifier Keys = true | false)
	//  calls to insertItem will set the current key to aDefaultKeyCode4.
	static bool useFourModifierKeys();
	static void useFourModifierKeys( bool b );
	static bool qtSupportsMetaKey();

	// Return the keyMod mask containing the bits set for the modifiers
	//  which may be used in accelerator shortcuts.
	static uint accelModMaskQt();		// Normally Qt::SHIFT | Qt::CTRL | Qt::ALT | (Qt::ALT<<1)

	// Returns true if X has the Meta key assigned to a modifier bit
	static bool keyboardHasMetaKey();

 // TODO: Decide whether to make these protected...
 public:
	enum Origin { OriginUnset, OriginQt, OriginNative /*OriginNativeCode, OriginNativeSym*/ };
	Origin m_origin;
	int m_keyCombQt;
	int m_keyCode, m_keySym, m_keyMod;
	int m_keyCombQtExplicit;
	int m_keySymExplicit, m_keyModExplicit;

	void calcKeyQt();
	static int compare( const KKeySequence& a, const KKeySequence& b );

 private:
	class KKeySequencePrivate* d;
};

class KKeySequences : public QValueVector<KKeySequence>
{
 public:
	KKeySequences() { }
	KKeySequences( const QString& sKey );

	KKeySequence first() const;
	bool operator ==( KKeySequences& keys );
};

// OBSOLETE: for backwards compatibility to KDE 2.2
class KKey : public KKeySequence
{
public:
	static QString keyToString( int k, bool bi18n = true )
		{ return KKeySequence(k).toString( bi18n ? I18N_Yes : I18N_No ); }
};

#endif // _KKEYSEQUENCE_H
