#ifndef _KKEYSERVER_X11_H
#define _KKEYSERVER_X11_H

#include "kshortcut.h"
#include "kkeynative.h"

namespace KKeyServer
{
	struct CodeMod	{ int code, mod; };

	struct Sym
	{
	 public:
		uint m_sym;

		Sym()
			{ m_sym = 0; }
		Sym( uint sym )
			{ m_sym = sym; }
		Sym( const QString& s )
			{ init( s ); }

		bool initQt( int keyQt );
		bool init( const QString& );

		int qt() const;
		QString toStringInternal() const;
		QString toString() const;

		uint getModsRequired() const;
		uint getSymVariation() const;

		operator uint() const { return m_sym; }
		Sym& operator =( uint sym ) { m_sym = sym; return *this; }

	 private:
		QString toString( bool bUserSpace ) const;

		static void capitalizeKeyname( QString& );
	};

	struct Key
	{
		enum { CODE_FOR_QT = 256 };

		uint m_code;
		uint m_mod;
		uint m_sym;

		bool init( const KKey& key, bool bQt );

		bool isNative() const { return m_code != CODE_FOR_QT; }

		uint code() const { return m_code; }
		uint mod() const { return m_mod; }
		uint sym() const { return m_sym; }
		int keyCodeQt() const { return (int) m_sym; }

		void setKeycodeQt( int keyQt )
			{ m_code = CODE_FOR_QT; m_sym = keyQt; }

		Key& operator =( const KKeyNative& key );
		int compare( const Key& ) const;

		bool operator ==( const Key& b ) const
			{ return compare( b ) == 0; }
		bool operator <( const Key& b ) const
			{ return compare( b ) < 0; }

		KKey key() const;
	};

	struct Variations
	{
		enum { MAX_VARIATIONS = 4 };

		Key m_rgkey[MAX_VARIATIONS];
		uint m_nVariations;

		Variations() { m_nVariations = 0; }

		void init( const KKey&, bool bQt );
		
		uint count() const { return m_nVariations; }
		const Key& key( uint i ) const { return m_rgkey[i]; }
	};

	bool initializeMods();

	/** Returns the equivalent X modifier mask of the given modifier flag. */
	uint modX( KKey::ModFlag );
	/**
	 * Returns true if the current keyboard layout supports the Win key.
	 * Specifically, whether the Super or Meta keys are assigned to an X modifier.
	 */
	bool keyboardHasWinKey();
	uint modXShift();
	uint modXLock();
	uint modXCtrl();
	uint modXAlt();
	uint modXNumLock();
	uint modXWin();
	uint modXScrollLock();
	/** Returns bitwise OR'ed mask containing Shift, Ctrl, Alt, and
	 * Win (if available). */
	uint accelModMaskX();

	bool keyQtToSym( int keyQt, uint& sym );
	bool keyQtToMod( int keyQt, uint& mod );
	bool symToKeyQt( uint sym, int& keyQt );

	bool modToModQt( uint mod, int& modQt );
	bool modToModX( uint mod, uint& modX );
	bool modXToModQt( uint modX, int& modQt );
	bool modXToMod( uint modX, uint& mod );

	bool codeXToSym( uchar codeX, uint modX, uint& symX );

	QString modToStringInternal( uint mod );
	QString modToStringUser( uint mod );

	bool stringToSymMod( const QString&, uint& sym, uint& mod );

	void keyQtToKeyX( uint keyCombQt, unsigned char *pKeyCodeX, uint *pKeySymX, uint *pKeyModX );
};

#endif // !_KKEYSERVER_X11_H
