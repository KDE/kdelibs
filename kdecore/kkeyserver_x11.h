#ifndef _KKEYSERVER_X11_H
#define _KKEYSERVER_X11_H

#include "kshortcut.h"

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

	operator uint() const { return m_sym; }
	Sym& operator =( uint sym ) { m_sym = sym; return *this; }

private:
	QString toString( bool bUserSpace ) const;

	static void capitalizeKeyname( QString& );
};

/*	struct SymMod
	{
		uint m_sym, m_mod;

		SymMod( int sym, int mod )
			{ m_sym = sym; m_mod = mod; }

		// This sets m_sym = sym, and sets mod to any modifiers required
		//  in order to activate that key.
		// getModsRequiredForSym( int sym ) & KKeyX11::symInfoPtr( sym );
		//void init( uint sym );
		// KKeyX11::keyCodeXToKeySymX( (uchar) code, (uint) mod );
		void initX( uchar codeX, uint modX );
		// KKeySequenceOlds( QString )
		bool init( const QString& );

		// KKeyX11::keySymXToKeyQt( sym, 0 );
		int qt() const;

		// KKeyX11::keySymXToString( symNative, 0, true );
		QString toStringInternal() const;
		QString toStringUser() const;
	};
*/
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
	uint getSymVariation( uint sym );

	bool keyQtToSym( int keyQt, uint& sym );
	bool keyQtToMod( int keyQt, uint& mod );
	bool symToKeyQt( uint sym, int& keyQt );

	bool modToModQt( uint mod, int& modQt );
	bool modToModX( uint mod, uint& modX );
	bool modXToModQt( uint modX, int& modQt );
	bool modXToMod( uint modX, uint& mod );

	bool codeXToSym( uchar codeX, uint modX, uint& symX );

	QString symToStringInternal( uint sym );
	QString symToStringUser( uint sym );
	QString modToStringInternal( uint mod );
	QString modToStringUser( uint mod );

	bool stringToSymMod( const QString&, uint& sym, uint& mod );

	void keyQtToKeyX( uint keyCombQt, unsigned char *pKeyCodeX, uint *pKeySymX, uint *pKeyModX );
};

#endif // !_KKEYSERVER_X11_H
