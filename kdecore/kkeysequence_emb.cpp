#include "kkeysequence.h"
#include <kckey.h>
#include <kdebug.h>
#include <klocale.h>

#ifndef Q_WS_X11

QString KKeySequence::toString( KKeySequence::I18N bi18n ) const
{
	int keyCombQt = m_keyCombQt;
	uint keySymQt = keyCombQt & 0xffff;
	QString keyStr, keyModStr;

	//FIXME: This is just the basics, needs to become more advanced
	if(keySymQt && (Qt::ALT<<1))
		keyModStr += ((bi18n) ? i18n("Meta") : "Meta") + "+";
	if(keySymQt & Qt::ALT)
		keyModStr += ((bi18n) ? i18n("Alt") : "Alt") + "+";
	if(keySymQt & Qt::CTRL)
		keyModStr += ((bi18n) ? i18n("Ctrl") : "Ctrl") + "+";
	if(keySymQt & Qt::SHIFT)
		keyModStr = ((bi18n) ? i18n("Shift") : "Shift") + "+";

	keyStr = (bi18n) ? i18n("Unknown Key", "Unknown") : QString("Unknown");
	// Determine name of primary key.
	// If printable, non-space unicode character,
	//  then display it directly instead of by name
	//  (e.g. '!' instead of 'Exclam')
	// UNLESS we're not wanting internationalization.  Then all
	//  keys should be printed with their ASCII-7 name.
	if( bi18n && keySymQt < 0x1000 && QChar(keySymQt).isPrint() && !QChar(keySymQt).isSpace() )
		keyStr = QChar(keySymQt);
	else {
		for( int i = 0; i < NB_KEYS; i++ ) {
			if( keySymQt == (uint) KKEYS[i].code ) {
				if (bi18n)
					keyStr = i18n("QAccel", KKEYS[i].name);
				else
					keyStr = KKEYS[i].name;
				break;
			}
		}
	}

	return !keyStr.isEmpty() ? (keyModStr + keyStr) : QString::null;
}

KKeySequences KKeySequence::stringToKeys( QString keyStr )
{
	KKeySequences keys;
	//kdDebug(125) << QString("stringToKey("+keyStr+") = %1\n").arg(stringToKey( keyStr, 0, 0, 0 ), 0, 16);

	uint	keyModQt = 0, keySymQt = 0;
	QString sKeySym;
	QChar	c;

	if( keyStr.isNull() || keyStr.isEmpty() )
		return keys;

	keyStr = keyStr.lower();
	int iOffset = 0, iOffsetToken;
	do {
		int i;

		// Find next token.
		iOffsetToken = keyStr.find( '+', iOffset );
		// If no more '+'s are found, set to end of string.
		if( iOffsetToken < 0 )
			iOffsetToken = keyStr.length();
		// Allow a '+' to be the keysym if it's the last character.
		else if( iOffsetToken == iOffset && iOffset + 1 == (int)keyStr.length() )
			iOffsetToken++;
		sKeySym = keyStr.mid( iOffset, iOffsetToken - iOffset ).stripWhiteSpace();
		iOffset = iOffsetToken + 1;

		//FIXME(E): This needs to be refined
		if( keyStr == "shift" )
			keyModQt |= Qt::SHIFT;
		else if( keyStr == "ctrl" )
			keyModQt |= Qt::CTRL;
		else if( keyStr == "alt" )
			keyModQt |= Qt::ALT;
		else if( keyStr == "meta" )
			keyModQt |= (Qt::ALT<<1);

		// If this was not a modifier key,
		//  search for 'normal' key.
		else {
			// Break if 'normal' key already found.
			if( keySymQt ) {
				keySymQt = 0;
				keyModQt = 0;
				break;
			}

			if( sKeySym.length() == 1 )
				c = sKeySym[0];
			else {
				// Search for Qt keycode
				for( i = 0; i < NB_KEYS; i++ ) {
					if( qstricmp( sKeySym.ascii(), KKEYS[i].name ) == 0 ) {
						keySymQt = KKEYS[i].code;
						if( KKEYS[i].code < 0x1000 && QChar(KKEYS[i].code).isLetter() )
							c = KKEYS[i].code;
						break;
					}
				}
			}
			break;
		}
	} while( (uint)iOffsetToken < keyStr.length() );

	if( !c.isNull() ) {
		if( c.isLetter() && (keyModQt & Qt::SHIFT) != Qt::SHIFT )
			c = c.lower();

		// For some reason, Qt always wants 'a-z' as 'A-Z'.
		if( c >= 'a' && c <= 'z' )
			c = c.upper();
		keySymQt = c.unicode();
	}

	if( !keySymQt )
		keyModQt = 0;

	KKeySequence key;
	key.m_origin = OriginQt;
	key.m_keyCombQt = keyModQt | keySymQt;
	key.m_keyCombQtExplicit = keyModQt | keySymQt;
	keys.push_back( key );

	return keys;
}

bool KKeySequence::keyboardHasMetaKey()
{
	//FIXME(E): Is there any way to implement this in Qt/Embedded?
	return false;
}

void KKeySequence::calcKeyQt()
{
}

#endif // !Q_WS_X11
