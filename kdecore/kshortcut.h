/* This file is part of the KDE libraries
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

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

#ifndef __KSHORTCUT_H
#define __KSHORTCUT_H

#include <qkeysequence.h>
#include <qstring.h>

class QKeyEvent;
typedef union  _XEvent XEvent;

class KKey;
class KKeyNative;
class KKeyVariations;
class KKeySequence;
class KShortcut;

class KKey
{
 public:
	enum { MOD_FLAG_COUNT = 4 };
	enum { QtWIN = (Qt::ALT << 1) };
	enum ModFlag {
		SHIFT = 0x01,
		CTRL = 0x02,
		ALT = 0x04,
		WIN = 0x08
	};

	KKey();
	KKey( int keyQt );
	KKey( const QKeySequence& );
	KKey( const QKeyEvent* );
	KKey( const KKey& );
	KKey( const QString& );
	~KKey();

 // Initialization methods
	void clear();
	bool init( int keyQt );
	bool init( const QKeySequence& );
	bool init( const QKeyEvent* );
	bool init( const KKey& );
	bool init( const QString& );

	KKey& operator =( const KKey& spec )
		{ init( spec ); return *this; }

 // Query methods.
	bool isNull() const;
	bool isSetAndValid() const;

	int key() const;
	int modFlags() const;

 // Comparison Methods
	int compare( const KKey& ) const;
	bool operator == ( const KKey& spec ) const
		{ return compare( spec ) == 0; }
	bool operator != ( const KKey& spec ) const
		{ return compare( spec ) != 0; }
	bool operator < ( const KKey& spec ) const
		{ return compare( spec ) < 0; }

 // Conversion methods.
	int keyCodeQt() const;
	QString toString() const;
	QString toStringInternal() const;

	static KKey& null();

 protected:
	enum StatusFlag {
		SET = 0x01,
		VALID = 0x02
	};

	/**
	 * Under X11, m_key will hold an X11 key symbol.
	 * For Qt/Embedded, it will hold the Qt key code.
	 */
	int m_key;

	int m_mod;

	/**
	 * Contains the StatusFlags --
	 *  SET | VALID   A valid value has been set
	 *  SET           A setting was attempted which resulted in an error
	 *  <null>        Object has not yet been initialized
	 */
	int m_flags;

 private:
	class KKeyPrivate* d;

	KKey( int key, int mod );
	bool init( int key, int mod );

	friend class KKeyNative;
};

// Representation of a key in the format native the windowing system (i.e. X11).
class KKeyNative
{
 public:
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
	KKey spec() const;
	operator KKey() const     { return spec(); }

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
	static bool keyboardHasMetaKey();

	/**
	 * This function is used by KKey to get the native equivalent
	 *  to Qt keycodes.
	 */
	static bool keyQtToSym( int keyQt, int& sym );
	static bool symToKeyQt( int sym, int& keyQt );

	static bool keyToVariations( const KKey& spec, KKeyVariations& key );
	static QString symToStringInternal( int sym );
	static QString symToString( int sym );
	//static QString symToStringSub( int sym, bool bi18n );
	//static QString keyNativeToString( int keyNative );

	static bool stringToSym( const QString& sKey, int& sym, int& mod );

	static int modX( int modSpec );

 protected:
	int m_code, m_mod, m_sym;

 private:
	class KKeyNativePrivate* d;
};

// All variations of a given key.
// Ex: Ctrl+Plus => { Ctrl+Plus, Ctrl+KP_Plus }
class KKeyVariations
{
 public:
	enum { MAX_VARIATIONS = 4 };

	KKeyVariations();
	KKeyVariations( const KKey& );
	KKeyVariations( const KKeyVariations& );
	~KKeyVariations();

	void clear();
	bool init( const KKey& );
	bool init( const KKeyVariations& );

	KKeyVariations& operator =( const KKeyVariations& key )
		{ init( key ); return *this; }

	const KKey& spec() const;
	uint variationCount() const;
	uint variationNativeCount() const;
	const int variation( uint i ) const;
	const KKeyNative& variationNative( uint i ) const;

	bool isNull() const;
	int compare( const KKeyVariations& ) const;
	bool operator == ( const KKeyVariations& key ) const
		{ return compare( key ) == 0; }
	bool operator != ( const KKeyVariations& key ) const
		{ return compare( key ) != 0; }
	bool operator < ( const KKeyVariations& key ) const
		{ return compare( key ) < 0; }

	QString toString() const;
	QString toStringInternal() const;

	static KKeyVariations& null();

 protected:
	KKey m_spec;
	uint m_nVariationsNative, m_nVariationsQt;
	KKeyNative m_rgkeyNative[MAX_VARIATIONS];
	int m_rgkeyQt[MAX_VARIATIONS];
	uint m_fSet;

 private:
	class KKeyPrivate* d;
	friend class KKeyNative;
};

// Ex: Ctrl+X,I
class KKeySequence
{
 public:
	enum { MAX_KEYS = 4 };

	KKeySequence();
	KKeySequence( const QKeySequence& );
	KKeySequence( const KKey& );
	KKeySequence( const KKeyNative& );
	KKeySequence( const KKeyVariations& );
	KKeySequence( const KKeySequence& );
	KKeySequence( const QString& );
	~KKeySequence();

	void clear();
	bool init( const QKeySequence& );
	bool init( const KKey& );
	bool init( const KKeyNative& );
	bool init( const KKeyVariations& );
	bool init( const KKeySequence& );
	bool init( const QString& );

	KKeySequence& operator =( const KKeySequence& seq )
		{ init( seq ); return *this; }

	uint count() const;
	const KKeyVariations& key( uint i ) const;
	bool isTriggerOnRelease() const { return m_bTriggerOnRelease; }

	void setTriggerOnRelease( bool );

	bool isNull() const;
	bool startsWith( const KKeySequence& ) const;
	int compare( const KKeySequence& ) const;
	bool operator == ( const KKeySequence& seq ) const
		{ return compare( seq ) == 0; }
	bool operator != ( const KKeySequence& seq ) const
		{ return compare( seq ) != 0; }
	bool operator < ( const KKeySequence& seq ) const
		{ return compare( seq ) < 0; }

	QKeySequence qt() const;
	QString toString() const;
	QString toStringInternal() const;

	static KKeySequence& null();

 protected:
	uint m_nKeys;
	KKeyVariations m_rgkey[MAX_KEYS];
	bool m_bTriggerOnRelease;

 private:
	class KKeySequencePrivate* d;
	friend class KKeyNative;
};

// Ex: Ctrl+V;Shift+Insert
class KShortcut
{
 public:
	enum { MAX_SEQUENCES = 2 };

	KShortcut();
	KShortcut( int keyQt );
	KShortcut( const QKeySequence& );
	KShortcut( const KKey& );
	KShortcut( const KKeyVariations& );
	KShortcut( const KKeySequence& );
	KShortcut( const KShortcut& );
	KShortcut( const QString& );
	~KShortcut();

	void clear();
	bool init( int keyQt );
	bool init( const QKeySequence& );
	bool init( const KKey& );
	bool init( const KKeyVariations& );
	bool init( const KKeySequence& );
	bool init( const KShortcut& );
	bool init( const QString& );

	KShortcut& operator =( const KShortcut& cut )
		{ init( cut ); return *this; }

	uint count() const;
	KKeySequence& seq( uint i );
	const KKeySequence& seq( uint i ) const;
	int keyCodeQt() const;
	QKeySequence keyPrimaryQt() const;
	KKeyNative keyPrimaryNative() const;

	bool isNull() const;
	int compare( const KShortcut& ) const;
	bool operator == ( const KShortcut& cut ) const
		{ return compare( cut ) == 0; }
	bool operator != ( const KShortcut& cut ) const
		{ return compare( cut ) != 0; }
	bool operator < ( const KShortcut& cut ) const
		{ return compare( cut ) < 0; }

	bool contains( const KKey& ) const;
	bool contains( const KKeySequence& ) const;
	bool insert( const KKeySequence& );

	operator QKeySequence () const;
	QString toString() const;
	QString toStringInternal( const KShortcut* pcutDefault = 0 ) const;

	static KShortcut& null();

 protected:
	uint m_nSeqs;
	KKeySequence m_rgseq[MAX_SEQUENCES];

 private:
	class KShortcutPrivate* d;
	friend class KKeyNative;

#ifndef KDE_NO_COMPAT
 public:
	operator int () const    { return keyCodeQt(); }
#endif
};

#endif // __KSHORTCUT_H
