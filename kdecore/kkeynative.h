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
	KKeyNative();
	KKeyNative( const XEvent* );
	KKeyNative( const KKey& );
	KKeyNative( const KKeyNative& );
	/** @internal */
	KKeyNative( uint code, uint mod, uint sym );
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

	uint code() const;
	uint mod() const;
	uint sym() const;

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
	static uint modX( KKey::ModFlag modFlag );

 private:
	uint m_code, m_mod, m_sym;
	class KKeyNativePrivate* d;
};

#endif // !__KKEYNATIVE_H
