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

// Representation of a key in the format native the windowing system (i.e. X11).
class KKeyNative
{
 public:
	class Variations
	{
		friend class KKeyNative;
	 public:
		enum { MAX_VARIATIONS = 4 };

		Variations() { m_nVariations = 0; }
		~Variations();

		void init( const KKey&, bool bQt );

		uint count() const { return m_nVariations; }
		const KKey& key( uint i ) const { return m_rgkey[i]; }

	 protected:
		uint m_nVariations;
		KKey m_rgkey[MAX_VARIATIONS];
	};

	KKeyNative();
	KKeyNative( const XEvent* );
	KKeyNative( const KKey& );
	KKeyNative( const KKeyNative& );
	~KKeyNative();

	void clear();
	bool init( const XEvent* );
	bool init( const KKey& );
	bool init( const KKeyNative& );

	KKeyNative& operator =( const KKeyNative& key )
		{ init( key ); return *this; }

	int keyCodeQt() const;
	KKey key() const;
	operator KKey() const     { return key(); }

	int code() const;
	int mod() const;
	int sym() const;

	bool isNull() const;
	int compare( const KKeyNative& ) const;
	bool operator == ( const KKeyNative& key ) const
		{ return compare( key ) == 0; }
	bool operator != ( const KKeyNative& key ) const
		{ return compare( key ) != 0; }
	bool operator < ( const KKeyNative& key ) const
		{ return compare( key ) < 0; }

	static KKeyNative& null();

	// General query functions. //
	static bool keyboardHasWinKey();

	/**
	 * This function is used by KKey to get the native equivalent
	 *  to Qt keycodes.
	 */
	static bool keyQtToSym( int keyQt, int& sym );
	static bool symToKeyQt( int sym, int& keyQt );

	static bool keyToVariations( const KKey& key, Variations& vkey );
	static QString symToStringInternal( int sym );
	static QString symToString( int sym );

	static bool stringToSym( const QString& sKey, int& sym, int& mod );

	static int modX( int modSpec );

 protected:
	int m_code, m_mod, m_sym;

 private:
	class KKeyNativePrivate* d;
};

#endif // !__KKEYNATIVE_H
