/* This file is part of the KDE libraries
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __KSHORTCUT_H
#define __KSHORTCUT_H

#include <qkeysequence.h>
#include <qstring.h>

class QKeyEvent;

class KKey;
class KKeyNative;
class KKeySequence;
class KShortcut;

/**
 * A KKey object represents a single key with possible modifiers
 * (Shift, Ctrl, Alt, Win).  It can represent both keys which are
 * understood by Qt as well as those which are additionally supported
 * by the underlying system (e.g. X11).
 */
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

	KKey& operator =( const KKey& key )
		{ init( key ); return *this; }

 // Query methods.
	bool isNull() const;

	int key() const;
	int modFlags() const;

 // Comparison Methods
	int compare( const KKey& ) const;
	bool operator == ( const KKey& key ) const
		{ return compare( key ) == 0; }
	bool operator != ( const KKey& key ) const
		{ return compare( key ) != 0; }
	bool operator < ( const KKey& key ) const
		{ return compare( key ) < 0; }

 // Conversion methods.
	int keyCodeQt() const;
	QString toString() const;
	QString toStringInternal() const;

	static KKey& null();

 protected:
	/**
	 * Under X11, m_key will hold an X11 key symbol.
	 * For Qt/Embedded, it will hold the Qt key code.
	 */
	int m_key;

	int m_mod;

 private:
	class KKeyPrivate* d;

	KKey( int key, int mod );
	bool init( int key, int mod );

	friend class KKeyNative;
};

/**
 * A KKeySequence object holds a sequence of up to 4 keys.
 * Ex: Ctrl+X,I
 */
class KKeySequence
{
 public:
	enum { MAX_KEYS = 4 };

	KKeySequence();
	KKeySequence( const QKeySequence& );
	KKeySequence( const KKey& );
	KKeySequence( const KKeyNative& );
	KKeySequence( const KKeySequence& );
	KKeySequence( const QString& );
	~KKeySequence();

	void clear();
	bool init( const QKeySequence& );
	bool init( const KKey& );
	bool init( const KKeyNative& );
	bool init( const KKeySequence& );
	bool init( const QString& );

	KKeySequence& operator =( const KKeySequence& seq )
		{ init( seq ); return *this; }

	uint count() const;
	const KKey& key( uint i ) const;
	bool isTriggerOnRelease() const;

	bool setKey( uint i, const KKey& );
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
	int keyCodeQt() const;
	QString toString() const;
	QString toStringInternal() const;

	static KKeySequence& null();

 protected:
	uchar m_nKeys;
	uchar m_bTriggerOnRelease;
	KKey m_rgvar[MAX_KEYS];

 private:
	class KKeySequencePrivate* d;
	friend class KKeyNative;
};

/**
 * The KShortcut class is used to represent a keyboard shortcut to an action.
 * A shortcut is normally a single key with modifiers, such as Ctrl+V.
 * A KShortcut object may also contain an alternate key which will also
 * activate the action it's associated to, as long as no other actions have
 * defined that key as their primary key.  Ex: Ctrl+V;Shift+Insert.
 */
class KShortcut
{
 public:
	enum { MAX_SEQUENCES = 2 };

	KShortcut();
	KShortcut( int keyQt );
	KShortcut( const QKeySequence& );
	KShortcut( const KKey& );
	KShortcut( const KKeySequence& );
	KShortcut( const KShortcut& );
	KShortcut( const char* );
	KShortcut( const QString& );
	~KShortcut();

	void clear();
	bool init( int keyQt );
	bool init( const QKeySequence& );
	bool init( const KKey& );
	bool init( const KKeySequence& );
	bool init( const KShortcut& );
	bool init( const QString& );

	KShortcut& operator =( const KShortcut& cut )
		{ init( cut ); return *this; }

	uint count() const;
	const KKeySequence& seq( uint i ) const;
	int keyCodeQt() const;

	bool isNull() const;
	int compare( const KShortcut& ) const;
	bool operator == ( const KShortcut& cut ) const
		{ return compare( cut ) == 0; }
	bool operator != ( const KShortcut& cut ) const
		{ return compare( cut ) != 0; }
	bool operator < ( const KShortcut& cut ) const
		{ return compare( cut ) < 0; }

	bool contains( const KKey& ) const;
	bool contains( const KKeyNative& ) const;
	bool contains( const KKeySequence& ) const;
	bool setSeq( uint i, const KKeySequence& );
	bool append( const KKeySequence& );

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
