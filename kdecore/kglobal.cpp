/*
* kglobal.cpp -- Implementation of class KGlobal.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sat May  1 02:08:43 EST 1999
*/

#include <assert.h>
#include "kglobal.h"

#include <kapp.h>

#include <kconfig.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kinstance.h>

#include <qfont.h>

KStandardDirs *KGlobal::dirs()
{
    return _instance->dirs();
}

KConfig	*KGlobal::config()
{
    return _instance->config();
}

KIconLoader *KGlobal::iconLoader()
{
    return _instance->iconLoader();
}

KLocale	*KGlobal::locale()
{	
    if( _locale == 0 ) {
	init();
	// will set _locale if it works - otherwise 0 is returned
	KLocale::initInstance();
    }
    
    return _locale;
}

KCharsets *KGlobal::charsets()
{
    if( _charsets == 0 ) {
	init();
	_charsets =new KCharsets();
    }
    
    return _charsets;
}

QFont KGlobal::generalFont()
{
    if(_generalFont) return *_generalFont;

    init();

    _generalFont = new QFont("helvetica", 12, QFont::Normal);
    charsets()->setQFont(*_generalFont, charsets()->charsetForLocale());
    KConfig *c = KGlobal::config();
    c->setGroup( "General" );
    *_generalFont = c->readFontEntry("font", _generalFont);
    return *_generalFont;
}
	
QFont KGlobal::fixedFont()
{
    if(_fixedFont) {
        return *_fixedFont;
    }

    init();

    KConfig *c = KGlobal::config();
    c->setGroup( "General" );
    _fixedFont = new QFont(c->readFontEntry("fixedFont"));

    if(!QFontInfo(*_fixedFont).fixedPitch() ) {
        *_fixedFont = QFont("fixed",
                            _fixedFont->pointSize(), QFont::Normal);
        _fixedFont->setStyleHint(QFont::Courier);
        _fixedFont->setFixedPitch(true);
        charsets()->setQFont(*_fixedFont, charsets()->charsetForLocale());
    }
    return *_fixedFont;
}

int KGlobal::dndEventDelay()
{
    static int delay = -1;

    init();

    if(delay == -1){
        KConfig *c = KGlobal::config();
        c->setGroup("General");
        delay = c->readNumEntry("DndDelay", 2);
    }
    return(delay);
}

void KGlobal::init()
{
    if (_instance)
	return;
    _instance = new KInstance("unknown");
    qAddPostRoutine( freeAll );
}

void KGlobal::freeAll()
{	
    delete _locale;
    _locale = 0;
    delete _charsets;
    _charsets = 0;
    delete _generalFont;
    _generalFont = 0;
    delete _fixedFont;
    _fixedFont = 0;
    delete _instance;
    _instance = 0;
}
	
// The Variables

KInstance       *KGlobal::_instance     = 0;
KLocale         *KGlobal::_locale	= 0;
KCharsets       *KGlobal::_charsets	= 0;

QFont           *KGlobal::_generalFont  = 0;
QFont           *KGlobal::_fixedFont    = 0;
